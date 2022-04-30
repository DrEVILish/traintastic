/**
 * server/src/hardware/interface/xpressnetinterface.hpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2019-2022 Reinder Feenstra
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

#ifndef TRAINTASTIC_SERVER_HARDWARE_INTERFACE_XPRESSNETINTERFACE_HPP
#define TRAINTASTIC_SERVER_HARDWARE_INTERFACE_XPRESSNETINTERFACE_HPP

#include "interface.hpp"
#include "../protocol/xpressnet/kernel.hpp"
#include "../protocol/xpressnet/settings.hpp"
#include "../decoder/decodercontroller.hpp"
#include "../decoder/list/decoderlist.hpp"
#include "../input/inputcontroller.hpp"
#include "../input/list/inputlist.hpp"
#include "../output/outputcontroller.hpp"
#include "../output/list/outputlist.hpp"
#include "../../core/objectproperty.hpp"
#include "../../enum/xpressnetinterfacetype.hpp"
#include "../../enum/xpressnetserialinterfacetype.hpp"
#include "../../enum/serialflowcontrol.hpp"

/**
 * @brief XpressNet hardware interface
 */
class XpressNetInterface final
  : public Interface
  , public DecoderController
  , public InputController
  , public OutputController
{
  CLASS_ID("interface.xpressnet")
  DEFAULT_ID("xpressnet")
  CREATE(XpressNetInterface)

  private:
    std::unique_ptr<XpressNet::Kernel> m_kernel;
    boost::signals2::connection m_xpressnetPropertyChanged;

    void addToWorld() final;
    void loaded() final;
    void destroying() final;
    void worldEvent(WorldState state, WorldEvent event) final;

    void idChanged(const std::string& newId) final;

    void updateVisible();

  protected:
    bool setOnline(bool& value, bool simulation) final;

  public:
    Property<XpressNetInterfaceType> type;
    Property<XpressNetSerialInterfaceType> serialInterfaceType;
    Property<std::string> device;
    Property<uint32_t> baudrate;
    Property<SerialFlowControl> flowControl;
    Property<std::string> hostname;
    Property<uint16_t> port;
    Property<uint8_t> s88StartAddress;
    Property<uint8_t> s88ModuleCount;
    ObjectProperty<XpressNet::Settings> xpressnet;
    ObjectProperty<DecoderList> decoders;
    ObjectProperty<InputList> inputs;
    ObjectProperty<OutputList> outputs;

    XpressNetInterface(World& world, std::string_view _id);

    // DecoderController:
    [[nodiscard]] bool addDecoder(Decoder& decoder) final;
    [[nodiscard]] bool removeDecoder(Decoder& decoder) final;
    void decoderChanged(const Decoder& decoder, DecoderChangeFlags changes, uint32_t functionNumber) final;

    // InputController:
    std::pair<uint32_t, uint32_t> inputAddressMinMax(uint32_t /*channel*/) const final { return {XpressNet::Kernel::ioAddressMin, XpressNet::Kernel::ioAddressMax}; }
    [[nodiscard]] bool addInput(Input& input) final;
    [[nodiscard]] bool removeInput(Input& input) final;
    void inputSimulateChange(uint32_t channel, uint32_t address) final;

    // OutputController:
    std::pair<uint32_t, uint32_t> outputAddressMinMax(uint32_t /*channel*/) const final { return {XpressNet::Kernel::ioAddressMin, XpressNet::Kernel::ioAddressMax}; }
    [[nodiscard]] bool addOutput(Output& output) final;
    [[nodiscard]] bool removeOutput(Output& output) final;
    [[nodiscard]] bool setOutputValue(uint32_t channel, uint32_t address, bool value) final;
};

#endif
