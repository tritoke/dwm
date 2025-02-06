/* See LICENSE file for copyright and license details. */

/* Modifer keys */
#include <X11/XF86keysym.h>

#define JETBRAINS_MONO "JetBrains Mono:size=9"
#define BERKELEY_MONO "TX\\-02:size=10"

/* appearance */
static const unsigned int borderpx  = 1;        /* border pixel of windows */
static const unsigned int snap      = 32;       /* snap pixel */
static const int swallowfloating    = 0;        /* 1 means swallow floating windows by default */
static const int showbar            = 1;        /* 0 means no bar */
static const int topbar             = 1;        /* 0 means bottom bar */
static const char *fonts[]          = { BERKELEY_MONO };
static const char dmenufont[]       = BERKELEY_MONO;

static const char col_base00[]      = "#121212"; /* darkest-grey */
static const char col_base01[]      = "#383830"; /* dark-grey */
static const char col_base02[]      = "#49483e"; /* darker-grey */
static const char col_base03[]      = "#75715e"; /* grey */
static const char col_base04[]      = "#a59f85"; /* light-grey-green */
static const char col_base05[]      = "#f8f8f2"; /* white (ish) */
static const char col_base06[]      = "#f5f4f1"; /* white (ish) */
static const char col_base07[]      = "#f9f8f5"; /* white (ish) */
static const char col_base08[]      = "#f92672"; /* vibrant pink */
static const char col_base09[]      = "#fd971f"; /* orange */
static const char col_base0A[]      = "#f4bf75"; /* beige-orange */
static const char col_base0B[]      = "#a6e22e"; /* vibrant lime green */
static const char col_base0C[]      = "#a1efe4"; /* light-aqua */
static const char col_base0D[]      = "#66d9ef"; /* light-blue */
static const char col_base0E[]      = "#ae81ff"; /* vibrant light-purple */
static const char col_base0F[]      = "#cc6633"; /* white (ish) */
static const char *colors[][3]      = {
  /*               fg         bg         border   */
  [SchemeNorm] = { col_base0D, col_base00, col_base0D },
  [SchemeSel]  = { col_base08, col_base00, col_base0D },
};

/* tagging */
static const char *tags[] = { "1", "2", "3", "4", "5", "6", "7", "8", "9" };

static const Rule rules[] = {
	/* xprop(1):
	 *	WM_CLASS(STRING) = instance, class
	 *	WM_NAME(STRING) = title
	 *
	 * For pesky Java apps that are very rude:
	 *   export _JAVA_AWT_WM_NONREPARENTING=1
	 *
	 * put this line in /etc/profile.d/jre.sh
	 * and then source it or login again.
	 *   - essential for Ghidra and Burpsuite
	 */

	/* class                     instance  title           tags mask  isfloating  isterminal  noswallow  monitor */
	{ "st",                      NULL,     NULL,           0,         0,          1,          -1,        -1 },
	{ "kitty",                   NULL,     NULL,           0,         0,          1,          -1,        -1 },
	{ "firefoxdeveloperedition", NULL,     NULL,           1 << 8,    0,          0,          -1,        -1 },
	{ "discord",                 NULL,     NULL,           1 << 7,    0,          0,           0,        -1 },
	{ "Spotify",                 NULL,     NULL,           1 << 6,    0,          0,           0,        -1 },
	{ "thunderbird",             NULL,     NULL,           1 << 5,    0,          0,           0,        -1 },
	{ NULL,                      NULL,     "Event Tester", 0,         1,          0,           1,        -1 }, /* xev */
};

/* layout(s) */
static const float mfact     = 0.5;  /* factor of master area size [0.05..0.95] */
static const int nmaster     = 1;    /* number of clients in master area */
static const int resizehints = 0;    /* 1 means respect size hints in tiled resizals */
static const int lockfullscreen = 1; /* 1 will force focus on the fullscreen window */

static const Layout layouts[] = {
	/* symbol     arrange function */
	{ "[]=",      tile },    /* first entry is default */
	{ "><>",      NULL },    /* no layout function means floating behavior */
	{ "[M]",      monocle },
	{ "|||",      col },
};

/* key definitions */
#define MODKEY Mod1Mask
#define WINKEY Mod4Mask
#define TAGKEYS(KEY,TAG) \
	{ MODKEY,                       KEY,      comboview,      {.ui = 1 << TAG} }, \
	{ MODKEY|ControlMask,           KEY,      toggleview,     {.ui = 1 << TAG} }, \
	{ MODKEY|ShiftMask,             KEY,      combotag,       {.ui = 1 << TAG} }, \
	{ MODKEY|ControlMask|ShiftMask, KEY,      toggletag,      {.ui = 1 << TAG} },

/* helper for spawning shell commands in the pre dwm-5.0 fashion */
#define SHCMD(cmd) { .v = (const char*[]){ "/bin/sh", "-c", cmd, NULL } }

#define DBUS_SEND_SPOTIFY(x) { .v = (const char*[]){ "dbus-send", "--print-reply", "--dest=org.mpris.MediaPlayer2.spotify", "/org/mpris/MediaPlayer2", "org.mpris.MediaPlayer2.Player." x, NULL } }
#define SCRIPT(x)            { .v = (const char*[]){ "/home/tritoke/.scripts/" x, NULL } }
#define PACTL(cmd, x)        { .v = (const char*[]){ "pactl", cmd, "@DEFAULT_SINK@", x, NULL } }
#define VOLUME_UP(x)         PACTL("set-sink-volume", "+" #x "%")
#define VOLUME_DOWN(x)       PACTL("set-sink-volume", "-" #x "%")
#define TOGGLE_MUTE          PACTL("set-sink-mute", "toggle")

/* commands */
static char dmenumon[2]                 = "0"; /* component of dmenucmd, manipulated in spawn() */

static const char dmenu_highpriority[]  = "spotify,discord,firefox-developer-edition,wireshark,ghidra,google-chrome-stable,zoom,quasselclient,thunderbird,telegram-desktop";
static const char *dmenucmd[]           = { "dmenu_run", "-m", dmenumon, "-fn", dmenufont, "-nb", col_base00, "-nf", col_base0D, "-sb", col_base08, "-sf", col_base0D, "-hb", col_base0D, "-hf", col_base00, "-hp", dmenu_highpriority, NULL };
static const char *termcmd[]            = { "kitty", NULL };
static const char *st[]                 = { "st", NULL };
static const char *brightness_up[]      = { "xbacklight", "-inc", "5", NULL };
static const char *brightness_down[]    = { "xbacklight", "-dec", "5", NULL };
static const char *shutdown[]           = { "systemctl", "poweroff", NULL };
static const char *suspend[]            = { "systemctl", "suspend", NULL };
static const char *reboot[]             = { "systemctl", "reboot", NULL };
static const char *slock[]              = { "slock", NULL };
static const char *xkill[]              = { "xkill", NULL };
static const char *_1password[]         = { "1password", NULL };

static const Arg screenshot         = SHCMD("maim -su | tee ~/Pictures/last_sc.png | xclip -selection clipboard -t image/png");
static const Arg bt_connect         = SHCMD("bluetoothctl power on && (bluetoothctl connect 80:99:E7:8F:49:64 || bluetoothctl connect F8:4E:17:8E:CA:17)");
static const Arg bt_disconnect      = SHCMD("bluetoothctl disconnect && bluetoothctl power off");
static const Arg volume_up          = VOLUME_UP(1);
static const Arg volume_down        = VOLUME_DOWN(1);
static const Arg volume_up_big      = VOLUME_UP(10);
static const Arg volume_down_big    = VOLUME_DOWN(10);
static const Arg volume_mute        = TOGGLE_MUTE;
static const Arg spotify_play_pause = DBUS_SEND_SPOTIFY("PlayPause");
static const Arg spotify_stop       = DBUS_SEND_SPOTIFY("Stop");
static const Arg spotify_next       = DBUS_SEND_SPOTIFY("Next");
static const Arg spotify_prev       = DBUS_SEND_SPOTIFY("Previous");
static const Arg emoji_picker       = SCRIPT("emojipicker");
static const Arg gitmoji_picker     = SCRIPT("gitmoji_picker");
static const Arg screenshot_window  = SCRIPT("screenshot_window");

#include "movestack.c"
static const Key keys[] = {
	TAGKEYS(XK_1, 0)
	TAGKEYS(XK_2, 1)
	TAGKEYS(XK_3, 2)
	TAGKEYS(XK_4, 3)
	TAGKEYS(XK_5, 4)
	TAGKEYS(XK_6, 5)
	TAGKEYS(XK_7, 6)
	TAGKEYS(XK_8, 7)
	TAGKEYS(XK_9, 8)

	/* modifier                     key                       function        argument */
	{ MODKEY,                       XK_b,                     togglebar,      {0} },
	{ MODKEY,                       XK_j,                     focusstack,     {.i = +1 } },
	{ MODKEY,                       XK_k,                     focusstack,     {.i = -1 } },
	{ MODKEY,                       XK_i,                     incnmaster,     {.i = +1 } },
	{ MODKEY,                       XK_d,                     incnmaster,     {.i = -1 } },
	{ MODKEY|ShiftMask,             XK_d,                     spawn,          bt_disconnect },
	{ MODKEY,                       XK_h,                     setmfact,       {.f = -0.05} },
	{ MODKEY,                       XK_l,                     setmfact,       {.f = +0.05} },
	{ MODKEY|ShiftMask,             XK_j,                     movestack,      {.i = +1 } },
	{ MODKEY|ShiftMask,             XK_k,                     movestack,      {.i = -1 } },
	{ MODKEY|ShiftMask,             XK_h,                     setcfact,       {.f = +0.10} },
	{ MODKEY|ShiftMask,             XK_l,                     setcfact,       {.f = -0.10} },
	{ MODKEY|ShiftMask,             XK_o,                     setcfact,       {.f =  0.00} },
	{ MODKEY,                       XK_Return,                zoom,           {0} },
	{ MODKEY,                       XK_Tab,                   view,           {0} },
	{ MODKEY|ShiftMask,             XK_c,                     killclient,     {0} },
	{ MODKEY,                       XK_t,                     setlayout,      {.v = &layouts[0]} },
	{ MODKEY,                       XK_f,                     setlayout,      {.v = &layouts[1]} },
	{ MODKEY,                       XK_m,                     setlayout,      {.v = &layouts[2]} },
	{ MODKEY,                       XK_c,                     setlayout,      {.v = &layouts[3]} },
	{ MODKEY,                       XK_space,                 setlayout,      {0} },
	{ MODKEY|ShiftMask,             XK_space,                 togglefloating, {0} },
	{ MODKEY,                       XK_0,                     view,           {.ui = ~0 } },
	{ MODKEY|ShiftMask,             XK_0,                     tag,            {.ui = ~0 } },
	{ MODKEY,                       XK_comma,                 focusmon,       {.i = +1 } },
	{ MODKEY,                       XK_period,                focusmon,       {.i = -1 } },
	{ MODKEY|ShiftMask,             XK_comma,                 tagmon,         {.i = -1 } },
	{ MODKEY|ShiftMask,             XK_period,                tagmon,         {.i = +1 } },
	{ MODKEY|ControlMask,           XK_h,                     rotatetags,     {.i = -1 } },
	{ MODKEY|ControlMask,           XK_l,                     rotatetags,     {.i = +1 } },
	// mirror windows bindings
	{ WINKEY|ControlMask,           XK_Left,                  rotatetags,     {.i = -1 } },
	{ WINKEY|ControlMask,           XK_Right,                 rotatetags,     {.i = +1 } },
	{ MODKEY|ShiftMask,             XK_q,                     quit,           {0} },
	{ MODKEY|ControlMask|ShiftMask, XK_q,                     quit,           {1} },
	{ MODKEY,                       XK_p,                     spawn,          {.v = dmenucmd } },
	{ MODKEY|ShiftMask,             XK_Return,                spawn,          {.v = termcmd } },
	{ MODKEY,                       XK_Return,                spawn,          {.v = termcmd } },
	{ MODKEY|ControlMask,           XK_Return,                spawn,          {.v = st } },
	{ MODKEY|ShiftMask,             XK_Delete,                spawn,          {.v = xkill } },
	{ MODKEY|ControlMask|ShiftMask, XK_s,                     spawn,          {.v = shutdown} },
	{ MODKEY|ControlMask|ShiftMask, XK_r,                     spawn,          {.v = reboot} },
	{ MODKEY|ControlMask|ShiftMask, XK_l,                     spawn,          {.v = slock } },
	{ MODKEY|ControlMask|ShiftMask, XK_u,                     spawn,          {.v = suspend} },
	{ ControlMask,                  XK_period,                spawn,          {.v = _1password} },
	{ 0,                            XF86XK_MonBrightnessUp,   spawn,          {.v = brightness_up } },
	{ 0,                            XF86XK_MonBrightnessDown, spawn,          {.v = brightness_down } },
	{ MODKEY,                       XK_Print,                 spawn,          screenshot },
	{ MODKEY|ShiftMask,             XK_s,                     spawn,          screenshot },
	{ MODKEY|ShiftMask,             XK_Print,                 spawn,          screenshot_window },
	{ MODKEY|ShiftMask,             XK_b,                     spawn,          bt_connect },
	{ MODKEY|ShiftMask,             XK_d,                     spawn,          bt_disconnect },
	{ MODKEY,                       XK_e,                     spawn,          emoji_picker},
	{ MODKEY,                       XK_g,                     spawn,          gitmoji_picker},
	{ 0,                            XF86XK_AudioRaiseVolume,  spawn,          volume_up_big },
	{ 0,                            XF86XK_AudioLowerVolume,  spawn,          volume_down_big },
	{ MODKEY,                       XF86XK_AudioRaiseVolume,  spawn,          volume_up },
	{ MODKEY,                       XF86XK_AudioLowerVolume,  spawn,          volume_down },
	{ ShiftMask,                    XF86XK_AudioRaiseVolume,  spawn,          volume_up },
	{ ShiftMask,                    XF86XK_AudioLowerVolume,  spawn,          volume_down },
	{ ControlMask,                  XF86XK_AudioRaiseVolume,  spawn,          volume_up },
	{ ControlMask,                  XF86XK_AudioLowerVolume,  spawn,          volume_down },
	{ 0,                            XF86XK_AudioMute,         spawn,          volume_mute },
	{ 0,                            XF86XK_AudioPlay,         spawn,          spotify_play_pause },
	{ 0,                            XF86XK_AudioPause,        spawn,          spotify_play_pause },
	{ 0,                            XF86XK_AudioStop,         spawn,          spotify_stop },
	{ 0,                            XF86XK_AudioNext,         spawn,          spotify_next },
	{ 0,                            XF86XK_AudioPrev,         spawn,          spotify_prev },
	{ MODKEY|ShiftMask,             XK_bracketleft,           spawn,          spotify_prev },
	{ MODKEY|ShiftMask,             XK_bracketright,          spawn,          spotify_next },
	{ MODKEY|ShiftMask,             XK_p,                     spawn,          spotify_play_pause },
};

/* button definitions */
/* click can be ClkTagBar, ClkLtSymbol, ClkStatusText, ClkWinTitle, ClkClientWin, or ClkRootWin */
static const Button buttons[] = {
	/* click                event mask      button          function        argument */
	{ ClkLtSymbol,          0,              Button1,        setlayout,      {0} },
	{ ClkLtSymbol,          0,              Button3,        setlayout,      {.v = &layouts[2]} },
	{ ClkWinTitle,          0,              Button2,        zoom,           {0} },
	{ ClkStatusText,        0,              Button2,        spawn,          {.v = termcmd} },
	{ ClkClientWin,         MODKEY,         Button1,        movemouse,      {0} },
	{ ClkClientWin,         MODKEY,         Button2,        togglefloating, {0} },
	{ ClkClientWin,         MODKEY,         Button3,        resizemouse,    {0} },
	{ ClkTagBar,            0,              Button1,        view,           {0} },
	{ ClkTagBar,            0,              Button3,        toggleview,     {0} },
	{ ClkTagBar,            MODKEY,         Button1,        tag,            {0} },
	{ ClkTagBar,            MODKEY,         Button3,        toggletag,      {0} },
};
