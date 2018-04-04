// license:BSD-3-Clause
// copyright-holders:Sven Schnelle
/***************************************************************************

  HP98644 Asynchronous serial interface

***************************************************************************/

#include "emu.h"
#include "hp98644.h"
#include "bus/rs232/rs232.h"

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE(HPDIO_98644, dio16_98644_device, "dio98644", "HP98644A Asynchronous Serial Interface")


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

MACHINE_CONFIG_START( dio16_98644_device::device_add_mconfig )
        MCFG_DEVICE_ADD(INS8250_TAG, INS8250, XTAL(2'457'600))

        MCFG_INS8250_OUT_TX_CB(DEVWRITELINE(RS232_TAG, rs232_port_device, write_txd))
        MCFG_INS8250_OUT_DTR_CB(DEVWRITELINE(RS232_TAG, rs232_port_device, write_dtr))
        MCFG_INS8250_OUT_RTS_CB(DEVWRITELINE(RS232_TAG, rs232_port_device, write_rts))

        MCFG_RS232_PORT_ADD(RS232_TAG, default_rs232_devices, nullptr)
        MCFG_RS232_RXD_HANDLER(DEVWRITELINE(INS8250_TAG, ins8250_uart_device, rx_w))
        MCFG_RS232_DCD_HANDLER(DEVWRITELINE(INS8250_TAG, ins8250_uart_device, dcd_w))
        MCFG_RS232_DSR_HANDLER(DEVWRITELINE(INS8250_TAG, ins8250_uart_device, dsr_w))
        MCFG_RS232_RI_HANDLER(DEVWRITELINE(INS8250_TAG, ins8250_uart_device, ri_w))
        MCFG_RS232_CTS_HANDLER(DEVWRITELINE(INS8250_TAG, ins8250_uart_device, cts_w))
MACHINE_CONFIG_END

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  dio16_98644_device - constructor
//-------------------------------------------------

dio16_98644_device::dio16_98644_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	dio16_98644_device(mconfig, HPDIO_98644, tag, owner, clock)
{
}

dio16_98644_device::dio16_98644_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, type, tag, owner, clock),
	device_dio16_card_interface(mconfig, *this),
	m_uart(*this, INS8250_TAG),
	m_switches{*this, "switches"}
{
}


#define REG_SWITCHES_REMOTE 0x01
#define REG_SWITCHES_98626_EN 0x02
#define REG_SWITCHES_MODEM_EN 0x04

#define REG_SWITCHES_SELECT_CODE_MASK 31
#define REG_SWITCHES_SELECT_CODE_SHIFT 5

#define REG_SWITCHES_INT_LEVEL_MASK 3
#define REG_SWITCHES_INT_LEVEL_SHIFT 3

static INPUT_PORTS_START(hp98644_port)
	PORT_START("switches")
	PORT_DIPNAME(REG_SWITCHES_REMOTE, 0x00, "Remote")
	PORT_DIPSETTING(0x00, DEF_STR(Off))
	PORT_DIPSETTING(REG_SWITCHES_REMOTE, DEF_STR(On))

	PORT_DIPNAME(REG_SWITCHES_98626_EN, 0x00, "98626 Emulation")
	PORT_DIPSETTING(0x00, DEF_STR(Off))
	PORT_DIPSETTING(REG_SWITCHES_98626_EN, DEF_STR(On))

	PORT_DIPNAME(REG_SWITCHES_MODEM_EN, 0x00, "Modem line enable")
	PORT_DIPSETTING(0x00, DEF_STR(Off))
	PORT_DIPSETTING(REG_SWITCHES_MODEM_EN, DEF_STR(On))

	PORT_DIPNAME(REG_SWITCHES_INT_LEVEL_MASK << REG_SWITCHES_INT_LEVEL_SHIFT, 0x00, "Interrupt level")
	PORT_DIPSETTING(0 << REG_SWITCHES_INT_LEVEL_SHIFT, "3")
	PORT_DIPSETTING(1 << REG_SWITCHES_INT_LEVEL_SHIFT, "4")
	PORT_DIPSETTING(2 << REG_SWITCHES_INT_LEVEL_SHIFT, "5")
	PORT_DIPSETTING(3 << REG_SWITCHES_INT_LEVEL_SHIFT, "6")

	PORT_DIPNAME(REG_SWITCHES_SELECT_CODE_MASK << REG_SWITCHES_SELECT_CODE_SHIFT, 0x09 << REG_SWITCHES_SELECT_CODE_SHIFT, "Select code")
	PORT_DIPSETTING(0 << REG_SWITCHES_SELECT_CODE_SHIFT, "0")
	PORT_DIPSETTING(1 << REG_SWITCHES_SELECT_CODE_SHIFT, "1")
	PORT_DIPSETTING(2 << REG_SWITCHES_SELECT_CODE_SHIFT, "2")
	PORT_DIPSETTING(3 << REG_SWITCHES_SELECT_CODE_SHIFT, "3")
	PORT_DIPSETTING(4 << REG_SWITCHES_SELECT_CODE_SHIFT, "4")
	PORT_DIPSETTING(5 << REG_SWITCHES_SELECT_CODE_SHIFT, "5")
	PORT_DIPSETTING(6 << REG_SWITCHES_SELECT_CODE_SHIFT, "6")
	PORT_DIPSETTING(7 << REG_SWITCHES_SELECT_CODE_SHIFT, "7")
	PORT_DIPSETTING(8 << REG_SWITCHES_SELECT_CODE_SHIFT, "8")
	PORT_DIPSETTING(9 << REG_SWITCHES_SELECT_CODE_SHIFT, "9")
	PORT_DIPSETTING(10 << REG_SWITCHES_SELECT_CODE_SHIFT, "10")
	PORT_DIPSETTING(11 << REG_SWITCHES_SELECT_CODE_SHIFT, "11")
	PORT_DIPSETTING(12 << REG_SWITCHES_SELECT_CODE_SHIFT, "12")
	PORT_DIPSETTING(13 << REG_SWITCHES_SELECT_CODE_SHIFT, "13")
	PORT_DIPSETTING(14 << REG_SWITCHES_SELECT_CODE_SHIFT, "14")
	PORT_DIPSETTING(15 << REG_SWITCHES_SELECT_CODE_SHIFT, "15")
	PORT_DIPSETTING(16 << REG_SWITCHES_SELECT_CODE_SHIFT, "16")
	PORT_DIPSETTING(17 << REG_SWITCHES_SELECT_CODE_SHIFT, "17")
	PORT_DIPSETTING(18 << REG_SWITCHES_SELECT_CODE_SHIFT, "18")
	PORT_DIPSETTING(19 << REG_SWITCHES_SELECT_CODE_SHIFT, "19")
	PORT_DIPSETTING(20 << REG_SWITCHES_SELECT_CODE_SHIFT, "20")
	PORT_DIPSETTING(21 << REG_SWITCHES_SELECT_CODE_SHIFT, "21")
	PORT_DIPSETTING(22 << REG_SWITCHES_SELECT_CODE_SHIFT, "22")
	PORT_DIPSETTING(23 << REG_SWITCHES_SELECT_CODE_SHIFT, "23")
	PORT_DIPSETTING(24 << REG_SWITCHES_SELECT_CODE_SHIFT, "24")
	PORT_DIPSETTING(25 << REG_SWITCHES_SELECT_CODE_SHIFT, "25")
	PORT_DIPSETTING(26 << REG_SWITCHES_SELECT_CODE_SHIFT, "26")
	PORT_DIPSETTING(27 << REG_SWITCHES_SELECT_CODE_SHIFT, "27")
	PORT_DIPSETTING(28 << REG_SWITCHES_SELECT_CODE_SHIFT, "28")
	PORT_DIPSETTING(29 << REG_SWITCHES_SELECT_CODE_SHIFT, "29")
	PORT_DIPSETTING(30 << REG_SWITCHES_SELECT_CODE_SHIFT, "30")
	PORT_DIPSETTING(31 << REG_SWITCHES_SELECT_CODE_SHIFT, "31")
INPUT_PORTS_END

ioport_constructor dio16_98644_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(hp98644_port);
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void dio16_98644_device::device_start()
{
	// set_nubus_device makes m_slot valid
	set_dio_device();
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void dio16_98644_device::device_reset()
{
	uint code = m_switches->read() >> REG_SWITCHES_SELECT_CODE_SHIFT;
	code &= REG_SWITCHES_SELECT_CODE_MASK;

	m_dio->install_memory(0x600000 + (code * 0x10000), 0x6007ff + (code * 0x10000), read16_delegate(FUNC(dio16_98644_device::io_r), this),
							write16_delegate(FUNC(dio16_98644_device::io_w), this));
}

READ16_MEMBER(dio16_98644_device::io_r)
{
	uint16_t ret = 0xffff;

	switch(offset) {
	case 0: /* ID */
	    ret = 0x02;
	    if (m_switches->read() & REG_SWITCHES_REMOTE)
		    ret |= 0x80;
	    if (m_switches->read() & REG_SWITCHES_98626_EN)
		    ret |= 0x40;
	    break;

	case 1:
	    ret = m_control | m_control << 8;
	    break;

	case 0x08:
	case 0x09:
	case 0x0a:
	case 0x0b:
	case 0x0c:
	case 0x0d:
	case 0x0e:
	case 0x0f:
		ret = m_uart->ins8250_r(space, offset & 0x07, mem_mask);
		break;
	}


	if (m_loopback && offset == 8) {
		ret = m_data;
	}
	return ret;
}

WRITE16_MEMBER(dio16_98644_device::io_w)
{
	if (offset == 0x0c)
		m_loopback = (data & 0x10) ? true : false;

	if (m_loopback && offset == 8) {
		m_data = data;
		return;
	}

	switch(offset) {
	case 0:
		device_reset();
		break;
	case 1:
		m_control = data & 0x80;
		break;

	case 0x08:
	case 0x09:
	case 0x0a:
	case 0x0b:
	case 0x0c:
	case 0x0d:
	case 0x0e:
	case 0x0f:
		m_uart->ins8250_w(space, offset & 0x07, data, mem_mask);
		break;
	}
}
