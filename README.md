YMACID
======

An OPL2 software groovebox that is suited for live use. It draws
inspiration from the Roland TB-303 and TR series and features two
x0x-like sequencers, one for drums and one for bass or lead.

Controls
--------

The controls have been optimized for a U.S. keyboard layout. It will
suck on non-qwerty layouts in particular.

    Common controls
    ---------------

    C-q            quit program
    return         start/stop playback
    tab            toggle between drum/bass modes
    =              increase pattern length
    -              decrease pattern length
    h or ←         move editor cursor left
    l or →         move editor cursor right
    H or C-←       shift pattern left
    L or C-→       shift pattern right
    pgup           increase tempo
    pgdn           decrease tempo
    F1-F8          change pattern (f1-f8)
    S-F1 to S-F8   change pattern (F1-F8)
    C-c            copy pattern
    C-v            paste pattern
    ;              clear step at cursor
    0              move cursor to first step
    o              repeat steps before cursor
    ,              decrease selected parameter value by 1
    .              increase selected parameter value by 1
    <              decrease selected parameter value by 4
    >              increase selected parameter value by 4

    Bass sequencer controls
    -------------

     2 3   5 6 7   set note at cursor
    q w e r t y u
    1              clear note at cursor
    4              randomize step at cursor
    f              toggle octave+ at cursor
    d              toggle octave- at cursor
    s              toggle slide at cursor
    a              toggle accent at cursor
    F              delete octave+ at cursor
    D              delete octave- at cursor
    S              delete slide at cursor
    A              delete accent at cursor
    k or ↑         increase pitch at cursor
    j or ↓         decrease pitch at cursor
    K or C-↑       increase pitch at and after cursor
    J or C-↓       increase pitch at and after cursor
    +              increase base pitch
    _              decrease base pitch

    Drum sequencer controls
    -----------------------

    1              toggle bass drum at cursor
    2              toggle snare drum at cursor
    3              toggle closed hi-hat at cursor
    4              toggle open hihat at cursor
    5              toggle low tom at cursor
    6              toggle high tom at cursor
    7              toggle cymbal at cursor
    8              toggle accent at cursor
    !              delete bass drum at cursor
    @              delete snare drum at cursor
    #              delete closed hi-hat at cursor
    $              delete open hihat at cursor
    %              delete low tom at cursor
    ^              delete high tom at cursor
    &              delete cymbal at cursor
    *              delete accent at cursor
    j or ↓         move cursor to next quarter note
    k or ↑         move cursor to previous quarter note
    Q              toggle bass drum mute
    W              toggle snare drum mute
    E              toggle hi-hat mute
    T              toggle toms mute
    U              toggle cymbal mute
    +              increase shuffle
    _              decrease shuffle
    ?              toggle shuffle between bass+drums and drums only

    Bass synthesizer parameters
    ---------------------------

    z              ML: modulator level
    x              MD: modulator decay
    c              MM: modulator harmonic
    v              MW: modulator waveform
    b              MS: modulator sustain level
    n              FB: modulator feedback factor
    Z              CL: carrier level
    X              CD: carrier decay
    C              CM: carrier harmonic
    V              CW: carrier waveform
    B              AL: accent depth

    Drum synthesizer parameters
    ---------------------------

    q             BC: bass drum click level
    w             BP: bass drum pitch
    e             BL: bass drum level
    r             BD: bass drum decay
    a             HL: hi-hat level
    s             HD: closed hi-hat decay
    d             SL: snare drum level
    f             SD: snare drum decay
    z             TL: tom level
    x             TD: tom decay
    c             TM: tom harmonic
    v             CL: cymbal level
    b             CD: cymbal decay

Building
--------

Prerequisites:

1.  Unix-like operating system
2.  dosbox (in \$PATH)
3.  unix2dos (in \$PATH)
4.  Turbo C 2.01
5.  GNU make (in \$PATH)

Launch dosbox and install Turbo C in the `dos` directory. When you are
done, the folder `dos/tc` should exist. Run GNU make in `$GITROOT`.

`build/YMACID.EXE` should appear.

Configuration
-------------

The program optionally uses a configuration file, YMACID.CFG. It
consists of whitespace delineated key and value pairs. For now:

-   port: the address of the OPL port
-   theme: the color theme to use, currently either "data" or "acid"
-   opl3: either 1 or 0. Enables OPL3 waveforms and optional splitting.
-   split: eiter 1 or 0. When set to 1, drums will pass through one
    channel and bass through the other
-   clock: either "int" or "pit" for now. "pit" mode uses the PC speaker
    timer as a clock source. This is very accurate, but does not work in
    DOSBox. "int" mode uses interrupt 0x15, function 0x8300 to setup
    delayed events at microsecond intervals. This works in DOSBox but is
    less accurate. "int" accuracy may depend on the motherboard chipset.

Example configuration:

    port 0x220
    theme data
    opl3 0
    split 0
    clock pit

That is also the default configuration if there is no configuration
file.

Compatibility notes
-------------------

The program may not run correctly in Windows 95 and will not run
correctly in dosbox using the PIT clock. It has been used successfully
with Windows 98, and Windows 95 in DOS mode. It should work with an
OPL2, but has only been tested with OPL3 in OPL2 compatibility mode.
