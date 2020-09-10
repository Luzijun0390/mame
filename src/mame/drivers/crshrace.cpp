// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
/***************************************************************************

Crash Race       (c) 1993 Video System Co.

driver by Nicola Salmoria

Notes:
- Keep player1 button 1 & 2 pressed while entering service mode to get an
  extended menu


Stephh's notes (based on the games M68000 code and some tests) :

0) all games

  - There seems to be preliminary support for 3 simultaneous players, but the
    game resets before the race starts if the 3 players don't play against each
    other! I can't tell however if it's an ingame or an emulation bug.
    To test this, change CRSHRACE_3P_HACK to 1, set the "Reset on P.O.S.T. Error"
    Dip Switch to "No" (because of the ROMS patch), and set the "Maximum Players"
    Dip Switch to "3".

  - There are 2 buttons for each player (one for accel and one for brake),
    the 3rd one being for "debug" purpose (see notes below).
  - The "Difficulty" Dip Switch also determines the time to complete the race.
  - "Coin B" Dip Switches only has an effect if you set the "Coin Slot"
    Dip Switch to "Same".
    If you set it to "Individual", it will use the coinage from "Coin A".
  - COIN3 adds 1 credit only if you set the "Coin Slot" Dip Switch to "Same".
    If you set it to "Individual", it will add 1 credit to fake player 3,
    thus having no effect.

  - DSW 3 bit 0 used to be a "Max Players" Dip Switch (but it is now unused) :
      * when Off, 2 players cabinet
      * when On,  3 players cabinet

  - DSW 3 bits 1 to 3 used to be a "Coin C" Dip Switch (but they are now unused)
    which is in fact similar to the table for "Coin A" and "Coin B" :
       1   2   3      Coinage
      Off Off Off      1C_1C
      On  Off Off      2C_1C
      Off On  Off      3C_1C
      On  On  Off      1C_2C
      Off Off On       1C_3C
      On  Off On       1C_4C
      Off On  On       1C_5C
      On  On  On       1C_6C

  - DSW 3 bit 7 is tested only if an error has occurred during P.O.S.T. :
      * when Off, the game is reset
      * when On,  don't bother with the error and continue

  - There are NO differences between Country code 0x0004 ("World") and 0x0005.
    Country code is stored at 0xfe1c9e and can have the following values :
      * 0000 : Japan
      * 0001 : USA & Canada
      * 0002 : Korea
      * 0003 : Hong Kong & Taiwan
      * 0004 : World
      * 0005 : ???

  - When in the "test mode" with the extended menu, pressing "P1 button 3"
    causes a "freeze"; press it again to unfreeze.
  - When in the "test mode" with the extended menu, pressing "P2 button 3"
    has an unknown effect (sound related ?), but sets bit 2 at 0xfe0019.

  - There are writes to 0xfff00c and 0xfff00d, but these addresses aren't mapped :
      * when "Flip Screen" Dip Switch is Off, 0x0001 is written to 0xfff00c.w
      * when "Flip Screen" Dip Switch is Off, 0xc001 is written to 0xfff00c.w
    I can't tell however what is the effect of these writes 8(


1) 'crshrace'

  - Even if there is code for it, there is NO possibility to select a 3 players
    game due to code at 0x003778 which "invalidates" the previous reading of DSW 3 :

    00363C: 13F8 F00B 00FE 1C85      move.b  $f00b.w, $fe1c85.l
    ...
    003650: 4639 00FE 1C85           not.b   $fe1c85.l
    ...
    003778: 51F9 00FE 1C85           sf      $fe1c85.l

  - When in the "test mode" with the extended menu, pressing "P1 start" +
    "P2 start" + the 3 buttons of the SAME player causes a reset of the game
    (code at 0x003182).
  - When in the "test play" menu of the "test mode", pressing "P1 button 1" +
    "P1 button 2" + "P2 button 1" + "P2 button 2" + "P2 button 3" returns
    to the "test mode" (code at 0x0040de).


2) 'crshrace2'

  - Even if there is code for it, there is NO possibility to select a 3 players
    game due to code at 0x003796 which "invalidates" the previous reading of DSW 3 :

    00365A: 13F8 F00B 00FE 1C85      move.b  $f00b.w, $fe1c85.l
    ...
    00366E: 4639 00FE 1C85           not.b   $fe1c85.l
    ...
    003796: 51F9 00FE 1C85           sf      $fe1c85.l

  - When in the "test mode" with the extended menu, pressing "P1 start" +
    "P2 start" + the 3 buttons of the SAME player causes a reset of the game
    (code at 0x0031a0).
  - When in the "test play" menu of the "test mode", pressing "P1 button 1" +
    "P1 button 2" + "P2 button 1" + "P2 button 2" + "P2 button 3" returns
    to the "test mode" (code at 0x0040fc).

  - I can't determine the effect of DSW 1 bit 4 8( All I can tell is that code
    at 0x00ea9c is called when initialising the race "parameters".


TODO:
- handle screen flip correctly
- sprite lag - I think it needs sprites to be delayed TWO frames
- is bg color in service mode right (blue)? Should it be black instead?
- handling of layer priority & enable might not be correct, though it should be
  enough to run this game.
- unknown writes to fff044/fff046. They look like two more scroll registers,
  but for what? The first starts at 0 when going over the start line and
  increases during the race

2008-08
Dip locations verified with Service Mode.

***************************************************************************/

#include "emu.h"
#include "includes/crshrace.h"

#include "cpu/m68000/m68000.h"
#include "sound/ymopn.h"
#include "screen.h"
#include "speaker.h"


#define CRSHRACE_3P_HACK    0


void crshrace_state::sh_bankswitch_w(uint8_t data)
{
	m_z80bank->set_entry(data & 0x03);
}


void crshrace_state::main_map(address_map &map)
{
	map(0x000000, 0x07ffff).rom();
	map(0x300000, 0x3fffff).rom().region("user1", 0);
	map(0x400000, 0x4fffff).rom().region("user2", 0).mirror(0x100000);
	map(0xa00000, 0xa0ffff).ram().share("spriteram.1");
	map(0xd00000, 0xd01fff).ram().w(FUNC(crshrace_state::videoram_w<0>)).share(m_videoram[0]);
	map(0xe00000, 0xe01fff).ram().share("spriteram.0");
	map(0xfe0000, 0xfeffff).ram();
	map(0xffc001, 0xffc001).w(FUNC(crshrace_state::roz_bank_w));
	map(0xffd000, 0xffdfff).ram().w(FUNC(crshrace_state::videoram_w<1>)).share(m_videoram[1]);
	map(0xffe000, 0xffefff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");
	map(0xfff001, 0xfff001).w(FUNC(crshrace_state::gfxctrl_w));
	map(0xfff000, 0xfff001).portr("P1");
	map(0xfff002, 0xfff003).portr("P2");
	map(0xfff004, 0xfff005).portr("DSW0");
	map(0xfff006, 0xfff007).portr("DSW2");
	map(0xfff009, 0xfff009).w(m_soundlatch, FUNC(generic_latch_8_device::write));
	map(0xfff00a, 0xfff00b).portr("DSW1");
	map(0xfff00e, 0xfff00f).portr("P3");
	map(0xfff020, 0xfff03f).w(m_k053936, FUNC(k053936_device::ctrl_w));
	map(0xfff044, 0xfff047).writeonly();   // ??? moves during race
}

void crshrace_state::sound_map(address_map &map)
{
	map(0x0000, 0x77ff).rom();
	map(0x7800, 0x7fff).ram();
	map(0x8000, 0xffff).bankr(m_z80bank);
}

void crshrace_state::sound_io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x00).w(FUNC(crshrace_state::sh_bankswitch_w));
	map(0x04, 0x04).rw(m_soundlatch, FUNC(generic_latch_8_device::read), FUNC(generic_latch_8_device::acknowledge_w));
	map(0x08, 0x0b).rw("ymsnd", FUNC(ym2610_device::read), FUNC(ym2610_device::write));
}


static INPUT_PORTS_START( crshrace )
	PORT_START("P1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)   // "Accel"
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)   // "Brake"
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_SERVICE2 )             // "Test"
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_TILT )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_SERVICE1 )

	PORT_START("P2")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)   // "Accel"
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)   // "Brake"
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW0")
	// DSW2 : 0xfe1c84 = !(0xfff005)
	PORT_DIPNAME( 0x0001, 0x0001, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW2:2")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0004, DEF_STR( Free_Play ) ) PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_SERVICE_DIPLOC( 0x0008, IP_ACTIVE_LOW, "SW2:4" )
	PORT_DIPUNUSED_DIPLOC( 0x0010, 0x0010, "SW2:5" )
	PORT_DIPUNUSED_DIPLOC( 0x0020, 0x0020, "SW2:6" )
	PORT_DIPNAME( 0x00c0, 0x00c0, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW2:7,8")
	PORT_DIPSETTING(      0x0080, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x00c0, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )
	// DSW1 : 0xfe1c83 = !(0xfff004)
	PORT_DIPNAME( 0x0100, 0x0100, "Coin Slot" ) PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(      0x0100, "Same" )
	PORT_DIPSETTING(      0x0000, "Individual" )
	PORT_DIPNAME( 0x0e00, 0x0e00, DEF_STR( Coin_A ) ) PORT_DIPLOCATION("SW1:2,3,4") PORT_CONDITION("DSW0", 0x0100, EQUALS, 0x0100)
	PORT_DIPSETTING(      0x0a00, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0c00, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0e00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0600, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x7000, 0x7000, DEF_STR( Coin_B ) ) PORT_DIPLOCATION("SW1:5,6,7") PORT_CONDITION("DSW0", 0x0100, EQUALS, 0x0100)
	PORT_DIPSETTING(      0x5000, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x6000, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x7000, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x3000, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x0e00, 0x0e00, DEF_STR( Coinage ) ) PORT_DIPLOCATION("SW1:2,3,4") PORT_CONDITION("DSW0", 0x0100, NOTEQUALS, 0x0100)
	PORT_DIPSETTING(      0x0a00, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0c00, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0e00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0600, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 1C_6C ) )
	PORT_DIPUNUSED_DIPLOC( 0x7000, 0x7000, "SW1:5,6,7") PORT_CONDITION("DSW0", 0x0100, NOTEQUALS, 0x0100)
	PORT_DIPNAME( 0x8000, 0x8000, "2 to Start, 1 to Cont." ) PORT_DIPLOCATION("SW1:8")  // Other desc. was too long !
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	PORT_START("DSW1")
	// DSW3 : 0xfe1c85 = !(0xfff00b)
#if CRSHRACE_3P_HACK
	PORT_DIPNAME( 0x0001, 0x0001, "Maximum Players" ) PORT_DIPLOCATION("SW3:1")
	PORT_DIPSETTING(      0x0001, "2" )
	PORT_DIPSETTING(      0x0000, "3" )
	PORT_DIPNAME( 0x000e, 0x000e, "Coin C" ) PORT_DIPLOCATION("SW3:2,3,4")
	PORT_DIPSETTING(      0x000a, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x000c, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x000e, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0006, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 1C_6C ) )
#else
	PORT_DIPUNUSED_DIPLOC( 0x0001, 0x0001, "SW3:1" )
	PORT_DIPUNUSED_DIPLOC( 0x0002, 0x0002, "SW3:2" )
	PORT_DIPUNUSED_DIPLOC( 0x0004, 0x0004, "SW3:3" )
	PORT_DIPUNUSED_DIPLOC( 0x0008, 0x0008, "SW3:4" )
#endif
	PORT_DIPUNUSED_DIPLOC( 0x0010, 0x0010, "SW3:5" )
	PORT_DIPUNUSED_DIPLOC( 0x0020, 0x0020, "SW3:6" )
	PORT_DIPUNUSED_DIPLOC( 0x0040, 0x0040, "SW3:7" )
	PORT_DIPNAME( 0x0080, 0x0080, "Reset on P.O.S.T. Error" ) PORT_DIPLOCATION("SW3:8") // Check code at 0x003812
	PORT_DIPSETTING(      0x0000, DEF_STR( No ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( Yes ) )

	PORT_START("P3")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(3)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(3)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(3)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_START3 )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x0f00, 0x0100, DEF_STR( Region ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( World ) )
	PORT_DIPSETTING(      0x0800, "USA & Canada" )
	PORT_DIPSETTING(      0x0000, DEF_STR( Japan ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( Korea ) )
	PORT_DIPSETTING(      0x0400, "Hong Kong & Taiwan" )
/*
    the following are all the same and seem to act like the World setting, possibly
    with a slightly different attract sequence
    PORT_DIPSETTING(      0x0300, "5" )
    PORT_DIPSETTING(      0x0500, "5" )
    PORT_DIPSETTING(      0x0600, "5" )
    PORT_DIPSETTING(      0x0700, "5" )
    PORT_DIPSETTING(      0x0900, "5" )
    PORT_DIPSETTING(      0x0a00, "5" )
    PORT_DIPSETTING(      0x0b00, "5" )
    PORT_DIPSETTING(      0x0c00, "5" )
    PORT_DIPSETTING(      0x0d00, "5" )
    PORT_DIPSETTING(      0x0e00, "5" )
    PORT_DIPSETTING(      0x0f00, "5" )
*/
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("soundlatch", generic_latch_8_device, pending_r)  // pending sound command
INPUT_PORTS_END

// Same as 'crshrace', but additional "unknown" Dip Switch (see notes)
static INPUT_PORTS_START( crshrace2 )
	PORT_INCLUDE( crshrace )

	PORT_MODIFY("DSW0")
	PORT_DIPUNKNOWN_DIPLOC( 0x0010, 0x0010, "SW2:5" )       // Check code at 0x00ea36
INPUT_PORTS_END



static const gfx_layout charlayout =
{
	8,8,
	RGN_FRAC(1,1),
	8,
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	{ 0*64, 1*64, 2*64, 3*64, 4*64, 5*64, 6*64, 7*64 },
	64*8
};

static const gfx_layout tilelayout =
{
	16,16,
	RGN_FRAC(1,1),
	4,
	{ 0, 1, 2, 3 },
	{ 2*4, 3*4, 0*4, 1*4, 6*4, 7*4, 4*4, 5*4,
			10*4, 11*4, 8*4, 9*4, 14*4, 15*4, 12*4, 13*4 },
	{ 0*64, 1*64, 2*64, 3*64, 4*64, 5*64, 6*64, 7*64,
			8*64, 9*64, 10*64, 11*64, 12*64, 13*64, 14*64, 15*64 },
	128*8
};

static const gfx_layout spritelayout =
{
	16,16,
	RGN_FRAC(1,1),
	4,
	{ 0, 1, 2, 3 },
	{ 1*4, 0*4, 3*4, 2*4, 5*4, 4*4, 7*4, 6*4,
			9*4, 8*4, 11*4, 10*4, 13*4, 12*4, 15*4, 14*4 },
	{ 0*64, 1*64, 2*64, 3*64, 4*64, 5*64, 6*64, 7*64,
			8*64, 9*64, 10*64, 11*64, 12*64, 13*64, 14*64, 15*64 },
	128*8
};

static GFXDECODE_START( gfx_crshrace )
	GFXDECODE_ENTRY( "chars",   0, charlayout,     0,  1 )
	GFXDECODE_ENTRY( "tiles",   0, tilelayout,   256, 16 )
	GFXDECODE_ENTRY( "sprites", 0, spritelayout, 512, 32 )
GFXDECODE_END


void crshrace_state::machine_start()
{
	m_z80bank->configure_entries(0, 4, memregion("audiocpu")->base(), 0x8000);

	save_item(NAME(m_roz_bank));
	save_item(NAME(m_gfxctrl));
	save_item(NAME(m_flipscreen));
}

void crshrace_state::machine_reset()
{
	m_roz_bank = 0;
	m_gfxctrl = 0;
	m_flipscreen = 0;
}

void crshrace_state::crshrace(machine_config &config)
{
	// basic machine hardware
	M68000(config, m_maincpu, 16000000);    // 16 MHz ???
	m_maincpu->set_addrmap(AS_PROGRAM, &crshrace_state::main_map);
	m_maincpu->set_vblank_int("screen", FUNC(crshrace_state::irq1_line_hold));

	Z80(config, m_audiocpu, 4000000);   // 4 MHz ???
	m_audiocpu->set_addrmap(AS_PROGRAM, &crshrace_state::sound_map);
	m_audiocpu->set_addrmap(AS_IO, &crshrace_state::sound_io_map);

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_size(64*8, 32*8);
	screen.set_visarea(0*8, 40*8-1, 0*8, 28*8-1);
	screen.set_screen_update(FUNC(crshrace_state::screen_update));
	screen.screen_vblank().set(m_spriteram[0], FUNC(buffered_spriteram16_device::vblank_copy_rising));
	screen.screen_vblank().append(m_spriteram[1], FUNC(buffered_spriteram16_device::vblank_copy_rising));
	screen.set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_crshrace);
	PALETTE(config, m_palette).set_format(palette_device::xGBR_555, 2048);

	VSYSTEM_SPR(config, m_spr, 0);
	m_spr->set_tile_indirect_cb(FUNC(crshrace_state::tile_callback));
	m_spr->set_gfx_region(2);
	m_spr->set_gfxdecode_tag(m_gfxdecode);

	BUFFERED_SPRITERAM16(config, m_spriteram[0]);
	BUFFERED_SPRITERAM16(config, m_spriteram[1]);

	K053936(config, m_k053936, 0);
	m_k053936->set_wrap(1);
	m_k053936->set_offsets(-48, -21);

	// sound hardware
	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	GENERIC_LATCH_8(config, m_soundlatch);
	m_soundlatch->data_pending_callback().set_inputline(m_audiocpu, INPUT_LINE_NMI);
	m_soundlatch->set_separate_acknowledge(true);

	ym2610_device &ymsnd(YM2610(config, "ymsnd", 8000000));
	ymsnd.irq_handler().set_inputline(m_audiocpu, 0);
	ymsnd.add_route(0, "lspeaker", 0.25);
	ymsnd.add_route(0, "rspeaker", 0.25);
	ymsnd.add_route(1, "lspeaker", 1.0);
	ymsnd.add_route(2, "rspeaker", 1.0);
}


ROM_START( crshrace )
	ROM_REGION( 0x80000, "maincpu", 0 ) // 68000 code
	ROM_LOAD16_WORD_SWAP( "1",            0x000000, 0x80000, CRC(21e34fb7) SHA1(be47b4a9bce2d6ce0a127dffe032c61547b2a3c0) )

	ROM_REGION16_BE( 0x100000, "user1", 0 )  // extra ROM
	ROM_LOAD16_WORD_SWAP( "w21",          0x000000, 0x100000, CRC(a5df7325) SHA1(614095a086164af5b5e73245744411187d81deec) )

	ROM_REGION16_BE( 0x100000, "user2", 0 )  // extra ROM
	ROM_LOAD16_WORD_SWAP( "w22",          0x000000, 0x100000, CRC(fc9d666d) SHA1(45aafcce82b668f93e51b5e4d092b1d0077e5192) )

	ROM_REGION( 0x20000, "audiocpu", 0 )    // 64k for the audio CPU + banks
	ROM_LOAD( "2",            0x00000, 0x20000, CRC(e70a900f) SHA1(edfe5df2dab5a7dccebe1a6f978144bcd516ab03) )

	ROM_REGION( 0x100000, "chars", 0 )
	ROM_LOAD( "h895",         0x000000, 0x100000, CRC(36ad93c3) SHA1(f68f229dd1a1f8bfd3b8f73b6627f5f00f809d34) )

	ROM_REGION( 0x400000, "tiles", 0 )
	ROM_LOAD( "w18",          0x000000, 0x100000, CRC(b15df90d) SHA1(56e38e6c40a02553b6b8c5282aa8f16b20779ebf) )
	ROM_LOAD( "w19",          0x100000, 0x100000, CRC(28326b93) SHA1(997e9b250b984b012ce1d165add59c741fb18171) )
	ROM_LOAD( "w20",          0x200000, 0x100000, CRC(d4056ad1) SHA1(4b45b14aa0766d7aef72f060e1cd28d67690d5fe) )
	// 300000-3fffff empty

	ROM_REGION( 0x400000, "sprites", 0 )
	ROM_LOAD( "h897",         0x000000, 0x200000, CRC(e3230128) SHA1(758c65f113481cf25bf0359deecd6736a7c9ee7e) )
	ROM_LOAD( "h896",         0x200000, 0x200000, CRC(fff60233) SHA1(56b4b708883a80761dc5f9184780477d72b80351) )

	ROM_REGION( 0x100000, "ymsnd:adpcmb", 0 ) // sound samples
	ROM_LOAD( "h894",         0x000000, 0x100000, CRC(d53300c1) SHA1(4c3ff7d3156791cb960c28845a5f1906605bce55) )

	ROM_REGION( 0x100000, "ymsnd:adpcma", 0 ) // sound samples
	ROM_LOAD( "h893",         0x000000, 0x100000, CRC(32513b63) SHA1(c4ede4aaa2611cedb53d47448422a1926acf3052) )
ROM_END

ROM_START( crshrace2 )
	ROM_REGION( 0x80000, "maincpu", 0 ) // 68000 code
	ROM_LOAD16_WORD_SWAP( "01-ic10.bin",  0x000000, 0x80000, CRC(b284aacd) SHA1(f0ef279cdec30eb32e8aa8cdd51e289b70f2d6f5) )

	ROM_REGION16_BE( 0x100000, "user1", 0 )  // extra ROM
	ROM_LOAD16_WORD_SWAP( "w21",          0x000000, 0x100000, CRC(a5df7325) SHA1(614095a086164af5b5e73245744411187d81deec) )    // IC14.BIN

	ROM_REGION16_BE( 0x100000, "user2", 0 )  // extra ROM
	ROM_LOAD16_WORD_SWAP( "w22",          0x000000, 0x100000, CRC(fc9d666d) SHA1(45aafcce82b668f93e51b5e4d092b1d0077e5192) )    // IC13.BIN

	ROM_REGION( 0x20000, "audiocpu", 0 )    // 64k for the audio CPU + banks
	ROM_LOAD( "2",            0x00000, 0x20000, CRC(e70a900f) SHA1(edfe5df2dab5a7dccebe1a6f978144bcd516ab03) )  // 02-IC58.BIN

	ROM_REGION( 0x100000, "chars", 0 )
	ROM_LOAD( "h895",         0x000000, 0x100000, CRC(36ad93c3) SHA1(f68f229dd1a1f8bfd3b8f73b6627f5f00f809d34) )    // IC50.BIN

	ROM_REGION( 0x400000, "tiles", 0 )
	ROM_LOAD( "w18",          0x000000, 0x100000, CRC(b15df90d) SHA1(56e38e6c40a02553b6b8c5282aa8f16b20779ebf) )    // ROM-A.BIN
	ROM_LOAD( "w19",          0x100000, 0x100000, CRC(28326b93) SHA1(997e9b250b984b012ce1d165add59c741fb18171) )    // ROM-B.BIN
	ROM_LOAD( "w20",          0x200000, 0x100000, CRC(d4056ad1) SHA1(4b45b14aa0766d7aef72f060e1cd28d67690d5fe) )    // ROM-C.BIN
	// 300000-3fffff empty

	ROM_REGION( 0x400000, "sprites", 0 )
	ROM_LOAD( "h897",         0x000000, 0x200000, CRC(e3230128) SHA1(758c65f113481cf25bf0359deecd6736a7c9ee7e) )    // IC29.BIN
	ROM_LOAD( "h896",         0x200000, 0x200000, CRC(fff60233) SHA1(56b4b708883a80761dc5f9184780477d72b80351) )    // IC75.BIN

	ROM_REGION( 0x100000, "ymsnd:adpcmb", 0 ) // sound samples
	ROM_LOAD( "h894",         0x000000, 0x100000, CRC(d53300c1) SHA1(4c3ff7d3156791cb960c28845a5f1906605bce55) )    // IC73.BIN

	ROM_REGION( 0x100000, "ymsnd:adpcma", 0 ) // sound samples
	ROM_LOAD( "h893",         0x000000, 0x100000, CRC(32513b63) SHA1(c4ede4aaa2611cedb53d47448422a1926acf3052) )    // IC69.BIN
ROM_END


#ifdef UNUSED_FUNCTION
void crshrace_state::patch_code(uint16_t offset)
{
	// A hack which shows 3 player mode in code which is disabled
	uint16_t *RAM = (uint16_t *)memregion("maincpu")->base();
	RAM[(offset + 0)/2] = 0x4e71;
	RAM[(offset + 2)/2] = 0x4e71;
	RAM[(offset + 4)/2] = 0x4e71;
}
#endif


void crshrace_state::init_crshrace()
{
	#if CRSHRACE_3P_HACK
	patch_code(0x003778);
	#endif
}

void crshrace_state::init_crshrace2()
{
	#if CRSHRACE_3P_HACK
	patch_code(0x003796);
	#endif
}


GAME( 1993, crshrace,  0,        crshrace, crshrace,  crshrace_state, init_crshrace,  ROT270, "Video System Co.", "Lethal Crash Race / Bakuretsu Crash Race (set 1)", MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )
GAME( 1993, crshrace2, crshrace, crshrace, crshrace2, crshrace_state, init_crshrace2, ROT270, "Video System Co.", "Lethal Crash Race / Bakuretsu Crash Race (set 2)", MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )
