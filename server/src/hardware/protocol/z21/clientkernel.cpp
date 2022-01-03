/**
 * server/src/hardware/protocol/z21/clientkernel.cpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2021-2022 Reinder Feenstra
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

#include "clientkernel.hpp"
#include "messages.hpp"
#include "../xpressnet/messages.hpp"
#include "../../decoder/decoder.hpp"
#include "../../decoder/decoderchangeflags.hpp"
#include "../../input/inputcontroller.hpp"
#include "../../../core/eventloop.hpp"
#include "../../../log/log.hpp"

namespace Z21 {

ClientKernel::ClientKernel(const ClientConfig& config)
  : m_keepAliveTimer(m_ioContext)
  , m_config{config}
{
}

void ClientKernel::setConfig(const ClientConfig& config)
{
  m_ioContext.post(
    [this, newConfig=config]()
    {
      m_config = newConfig;
    });
}

void ClientKernel::receive(const Message& message)
{
  if(m_config.debugLogRXTX)
    EventLoop::call(
      [this, msg=toString(message)]()
      {
        Log::log(m_logId, LogMessage::D2002_RX_X, msg);
      });

  switch(message.header())
  {
    case LAN_X:
    {
      const auto& lanX = static_cast<const LanX&>(message);

      if(!XpressNet::isChecksumValid(*reinterpret_cast<const XpressNet::Message*>(&lanX.xheader)))
        break;

      switch(lanX.xheader)
      {
        case 0x61:
          if(message == LanXBCTrackPowerOff())
          {
            if(m_trackPowerOn != TriState::False)
            {
              m_trackPowerOn = TriState::False;

              if(m_onTrackPowerOnChanged)
                EventLoop::call(
                  [this]()
                  {
                    m_onTrackPowerOnChanged(false);
                  });
            }
          }
          else if(message == LanXBCTrackPowerOn())
          {
            if(m_trackPowerOn != TriState::True)
            {
              m_trackPowerOn = TriState::True;

              if(m_onTrackPowerOnChanged)
                EventLoop::call(
                  [this]()
                  {
                    m_onTrackPowerOnChanged(true);
                  });
            }
          }
          break;

        case 0x81:
          if(message == LanXBCStopped())
          {
            if(m_emergencyStop != TriState::True)
            {
              m_emergencyStop = TriState::True;

              if(m_onEmergencyStop)
                EventLoop::call(
                  [this]()
                  {
                    m_onEmergencyStop();
                  });
            }
          }
          break;
      }
      break;
    }
    case LAN_GET_SERIAL_NUMBER:
      if(message.dataLen() == sizeof(LanGetSerialNumberReply))
      {
        const auto& reply = static_cast<const LanGetSerialNumberReply&>(message);

        if(m_serialNumber != reply.serialNumber())
        {
          m_serialNumber = reply.serialNumber();
          if(m_onSerialNumberChanged)
          {
            EventLoop::call(
              [this, serialNumber=m_serialNumber]()
              {
                m_onSerialNumberChanged(serialNumber);
              });
          }
        }
      }
      break;

    case LAN_GET_HWINFO:
      if(message.dataLen() == sizeof(LanGetHardwareInfoReply))
      {
        const auto& reply = static_cast<const LanGetHardwareInfoReply&>(message);

        if(m_hardwareType != reply.hardwareType() ||
            m_firmwareVersionMajor != reply.firmwareVersionMajor() ||
            m_firmwareVersionMinor != reply.firmwareVersionMinor())
        {
          m_hardwareType = reply.hardwareType();
          m_firmwareVersionMajor = reply.firmwareVersionMajor();
          m_firmwareVersionMinor = reply.firmwareVersionMinor();

          if(m_onHardwareInfoChanged)
          {
            EventLoop::call(
              [this, hardwareType=m_hardwareType, firmwareVersionMajor=m_firmwareVersionMajor, firmwareVersionMinor=m_firmwareVersionMinor]()
              {
                m_onHardwareInfoChanged(hardwareType, firmwareVersionMajor, firmwareVersionMinor);
              });
          }
        }
      }
      break;

    case LAN_GET_CODE:
    case LAN_LOGOFF:
    case LAN_SET_BROADCASTFLAGS:
    case LAN_GET_BROADCASTFLAGS:
    case LAN_GET_LOCO_MODE:
    case LAN_SET_LOCO_MODE:
    case LAN_GET_TURNOUTMODE:
    case LAN_SET_TURNOUTMODE:
    case LAN_RMBUS_DATACHANGED:
    case LAN_RMBUS_GETDATA:
    case LAN_RMBUS_PROGRAMMODULE:
    case LAN_SYSTEMSTATE_DATACHANGED:
    case LAN_SYSTEMSTATE_GETDATA:
    case LAN_RAILCOM_DATACHANGED:
    case LAN_RAILCOM_GETDATA:
    case LAN_LOCONET_Z21_RX:
    case LAN_LOCONET_Z21_TX:
    case LAN_LOCONET_FROM_LAN:
    case LAN_LOCONET_DISPATCH_ADDR:
    case LAN_LOCONET_DETECTOR:
    case LAN_CAN_DETECTOR:
      break; // not (yet) supported
  }
}

void ClientKernel::trackPowerOn()
{
  m_ioContext.post(
    [this]()
    {
      if(m_trackPowerOn != TriState::True || m_emergencyStop != TriState::False)
        send(LanXSetTrackPowerOn());
    });
}

void ClientKernel::trackPowerOff()
{
  m_ioContext.post(
    [this]()
    {
      if(m_trackPowerOn != TriState::False)
        send(LanXSetTrackPowerOff());
    });
}

void ClientKernel::emergencyStop()
{
  m_ioContext.post(
    [this]()
    {
      if(m_emergencyStop != TriState::True)
        send(LanXSetStop());
    });
}

void ClientKernel::decoderChanged(const Decoder& decoder, DecoderChangeFlags changes, uint32_t functionNumber)
{
  if(has(changes, DecoderChangeFlags::EmergencyStop | DecoderChangeFlags::Direction | DecoderChangeFlags::Throttle | DecoderChangeFlags::SpeedSteps))
  {
    LanXSetLocoDrive cmd;
    cmd.setAddress(decoder.address, decoder.longAddress);

    switch(decoder.speedSteps)
    {
      case 14:
      {
        const uint8_t speedStep = Decoder::throttleToSpeedStep(decoder.throttle, 14);
        cmd.db0 = 0x10;
        if(decoder.emergencyStop)
          cmd.speedAndDirection = 0x01;
        else if(speedStep > 0)
          cmd.speedAndDirection = speedStep + 1;
        break;
      }
      case 28:
      {
        uint8_t speedStep = Decoder::throttleToSpeedStep(decoder.throttle, 28);
        cmd.db0 = 0x12;
        if(decoder.emergencyStop)
          cmd.speedAndDirection = 0x01;
        else if(speedStep > 0)
        {
          speedStep++;
          cmd.speedAndDirection = ((speedStep & 0x01) << 4) | (speedStep >> 1);
        }
        break;
      }
      case 126:
      case 128:
      default:
      {
        const uint8_t speedStep = Decoder::throttleToSpeedStep(decoder.throttle, 126);
        cmd.db0 = 0x13;
        if(decoder.emergencyStop)
          cmd.speedAndDirection = 0x01;
        else if(speedStep > 0)
          cmd.speedAndDirection = speedStep + 1;
        break;
      }
    }

    if(decoder.direction.value() == Direction::Forward)
      cmd.speedAndDirection |= 0x80;

    cmd.checksum = XpressNet::calcChecksum(*reinterpret_cast<const XpressNet::Message*>(&cmd.xheader));
    postSend(cmd);
  }
  else if(has(changes, DecoderChangeFlags::FunctionValue))
  {
    if(functionNumber <= LanXSetLocoFunction::functionNumberMax)
      if(const auto& f = decoder.getFunction(functionNumber))
        postSend(LanXSetLocoFunction(
          decoder.address, decoder.longAddress,
          static_cast<uint8_t>(functionNumber),
          f->value ? LanXSetLocoFunction::SwitchType::On : LanXSetLocoFunction::SwitchType::Off));
  }
}

void ClientKernel::onStart()
{
  // reset all state values
  m_serialNumber = 0;
  m_hardwareType = HWT_UNKNOWN;
  m_firmwareVersionMajor = 0;
  m_firmwareVersionMinor = 0;
  m_trackPowerOn = TriState::Undefined;
  m_emergencyStop = TriState::Undefined;

  send(LanGetSerialNumber());
  send(LanGetHardwareInfo());

  send(LanSetBroadcastFlags(
    BroadcastFlags::PowerLocoTurnoutChanges |
    BroadcastFlags::SystemStatusChanges |
    BroadcastFlags::AllLocoChanges)); // seems not to work with DR5000

  send(LanGetBroadcastFlags());

  send(LanSystemStateGetData());

  startKeepAliveTimer();
}

void ClientKernel::onStop()
{
  send(LanLogoff());
}

void ClientKernel::send(const Message& message)
{
  if(m_ioHandler->send(message))
  {
    if(m_config.debugLogRXTX)
      EventLoop::call(
        [this, msg=toString(message)]()
        {
          Log::log(m_logId, LogMessage::D2001_TX_X, msg);
        });
  }
  else
  {} // log message and go to error state
}

void ClientKernel::startKeepAliveTimer()
{
  assert(ClientConfig::keepAliveInterval > 0);
  m_keepAliveTimer.expires_after(boost::asio::chrono::seconds(ClientConfig::keepAliveInterval));
  m_keepAliveTimer.async_wait(std::bind(&ClientKernel::keepAliveTimerExpired, this, std::placeholders::_1));
}

void ClientKernel::keepAliveTimerExpired(const boost::system::error_code& ec)
{
  if(ec)
    return;

  send(LanSystemStateGetData());

  startKeepAliveTimer();
}

}
