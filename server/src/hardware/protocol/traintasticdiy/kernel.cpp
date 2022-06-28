/**
 * server/src/hardware/protocol/traintasticdiy/kernel.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2022 Reinder Feenstra
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "kernel.hpp"
#include "messages.hpp"
#include "../../decoder/decoder.hpp"
#include "../../decoder/decoderchangeflags.hpp"
#include "../../input/inputcontroller.hpp"
#include "../../output/outputcontroller.hpp"
#include "../../../utils/inrange.hpp"
#include "../../../utils/setthreadname.hpp"
#include "../../../core/eventloop.hpp"
#include "../../../log/log.hpp"

namespace TraintasticDIY {

constexpr TriState toTriState(InputState value)
{
  switch(value)
  {
    case InputState::False:
      return TriState::False;

    case InputState::True:
      return TriState::True;

    case InputState::Undefined:
    case InputState::Invalid:
      break;
  }
  return TriState::Undefined;
}

constexpr TriState toTriState(OutputState value)
{
  switch(value)
  {
    case OutputState::False:
      return TriState::False;

    case OutputState::True:
      return TriState::True;

    case OutputState::Undefined:
    case OutputState::Invalid:
      break;
  }
  return TriState::Undefined;
}

Kernel::Kernel(const Config& config, bool simulation)
  : m_ioContext{1}
  , m_simulation{simulation}
  , m_heartbeatTimeout{m_ioContext}
  , m_inputController{nullptr}
  , m_outputController{nullptr}
  , m_config{config}
#ifndef NDEBUG
  , m_started{false}
#endif
{
}

void Kernel::setConfig(const Config& config)
{
  m_ioContext.post(
    [this, newConfig=config]()
    {
      m_config = newConfig;
    });
}

void Kernel::start()
{
  assert(m_ioHandler);
  assert(!m_started);

  m_featureFlagsSet = false;
  m_featureFlags1 = FeatureFlags1::None;
  m_featureFlags2 = FeatureFlags2::None;
  m_featureFlags3 = FeatureFlags3::None;
  m_featureFlags4 = FeatureFlags4::None;

  m_thread = std::thread(
    [this]()
    {
      setThreadName("traintasticdiy");
      auto work = std::make_shared<boost::asio::io_context::work>(m_ioContext);
      m_ioContext.run();
    });

  m_ioContext.post(
    [this]()
    {
      m_ioHandler->start();

      send(GetInfo());
      send(GetFeatures());

      restartHeartbeatTimeout();

      if(m_onStarted)
        EventLoop::call(
          [this]()
          {
            m_onStarted();
          });
    });

#ifndef NDEBUG
  m_started = true;
#endif
}

void Kernel::stop()
{
  m_ioContext.post(
    [this]()
    {
      m_heartbeatTimeout.cancel();
      m_ioHandler->stop();
    });

  m_ioContext.stop();

  m_thread.join();

#ifndef NDEBUG
  m_started = false;
#endif
}

void Kernel::receive(const Message& message)
{
  if(m_config.debugLogRXTX && (message != Heartbeat() || m_config.debugLogHeartbeat))
    EventLoop::call(
      [this, msg=toString(message)]()
      {
        Log::log(m_logId, LogMessage::D2002_RX_X, msg);
      });

  restartHeartbeatTimeout();

  switch(message.opCode)
  {
    case OpCode::Heartbeat:
      break;

    case OpCode::SetInputState:
    {
      if(!m_featureFlagsSet || !hasFeatureInput())
        break;

      const auto& setInputState = static_cast<const SetInputState&>(message);
      const uint16_t address = setInputState.address();
      if(inRange(address, ioAddressMin, ioAddressMax))
      {
        auto it = m_inputValues.find(address);
        if(it == m_inputValues.end() || it->second != setInputState.state)
        {
          m_inputValues[address] = setInputState.state;

          EventLoop::call(
            [this, address, state=setInputState.state]()
            {
              if(state == InputState::Invalid)
              {
                if(m_inputController->inputs().count({InputController::defaultInputChannel, address}) != 0)
                  Log::log(m_logId, LogMessage::W2004_INPUT_ADDRESS_X_IS_INVALID, address);
              }
              else
                m_inputController->updateInputValue(InputController::defaultInputChannel, address, toTriState(state));
            });
        }
      }
      break;
    }
    case OpCode::SetOutputState:
    {
      if(!m_featureFlagsSet || !hasFeatureOutput())
        break;

      const auto& setOutputState = static_cast<const SetOutputState&>(message);
      const uint16_t address = setOutputState.address();
      if(inRange(address, ioAddressMin, ioAddressMax))
      {
        auto it = m_outputValues.find(address);
        if(it == m_outputValues.end() || it->second != setOutputState.state)
        {
          m_outputValues[address] = setOutputState.state;

          EventLoop::call(
            [this, address, state=setOutputState.state]()
            {
              if(state == OutputState::Invalid)
              {
                if(m_outputController->outputs().count({OutputController::defaultOutputChannel, address}) != 0)
                  Log::log(m_logId, LogMessage::W2005_OUTPUT_ADDRESS_X_IS_INVALID, address);
              }
              else
                m_outputController->updateOutputValue(OutputController::defaultOutputChannel, address, toTriState(state));
            });
        }
      }
      break;
    }
    case OpCode::Features:
    {
      const auto& features = static_cast<const Features&>(message);
      m_featureFlagsSet = true;
      m_featureFlags1 = features.featureFlags1;
      m_featureFlags2 = features.featureFlags2;
      m_featureFlags3 = features.featureFlags3;
      m_featureFlags4 = features.featureFlags4;

      if(hasFeatureInput())
        EventLoop::call(
          [this]()
          {
            for(const auto& it : m_inputController->inputs())
              postSend(GetInputState(static_cast<uint16_t>(it.first.address)));
          });

      if(hasFeatureOutput())
        EventLoop::call(
          [this]()
          {
            for(const auto& it : m_outputController->outputs())
              postSend(GetOutputState(static_cast<uint16_t>(it.first.address)));
          });
      break;
    }
    case OpCode::Info:
    {
      const auto& info = static_cast<const InfoBase&>(message);
      EventLoop::call(
        [this, text=std::string(info.text())]()
        {
          Log::log(m_logId, LogMessage::I2005_X, text);
        });
      break;
    }
    case OpCode::GetInfo:
    case OpCode::GetFeatures:
    case OpCode::GetOutputState:
    case OpCode::GetInputState:
      assert(false);
      break;
  }
}

bool Kernel::setOutput(uint16_t address, bool value)
{
  postSend(SetOutputState(address, value ? OutputState::True : OutputState::False));
  return true;
}

void Kernel::simulateInputChange(uint16_t address)
{
  if(m_simulation)
    m_ioContext.post(
      [this, address]()
      {
        auto it = m_inputValues.find(address);
        receive(SetInputState(address, (it == m_inputValues.end() && it->second == InputState::True) ? InputState::False : InputState::True));
      });
}

void Kernel::setIOHandler(std::unique_ptr<IOHandler> handler)
{
  assert(handler);
  assert(!m_ioHandler);
  m_ioHandler = std::move(handler);
}

void Kernel::send(const Message& message)
{
  if(m_ioHandler->send(message))
  {
    if(m_config.debugLogRXTX && (message != Heartbeat() || m_config.debugLogHeartbeat))
      EventLoop::call(
        [this, msg=toString(message)]()
        {
          Log::log(m_logId, LogMessage::D2001_TX_X, msg);
        });
  }
  else
  {} // log message and go to error state
}

void Kernel::restartHeartbeatTimeout()
{
  m_heartbeatTimeout.expires_after(m_config.heartbeatTimeout);
  m_heartbeatTimeout.async_wait(std::bind(&Kernel::heartbeatTimeoutExpired, this, std::placeholders::_1));
}

void Kernel::heartbeatTimeoutExpired(const boost::system::error_code& ec)
{
  if(ec)
    return;
  m_heartbeatTimeout.cancel();
  send(Heartbeat());
  restartHeartbeatTimeout();
}

}
