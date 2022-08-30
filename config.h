/* See LICENSE file for copyright and license details. */

/* Modifer keys */
#include <X11/XF86keysym.h>

/* appearance */
static const unsigned int borderpx  = 1;        /* border pixel of windows */
static const unsigned int snap      = 32;       /* snap pixel */
static const int swallowfloating    = 0;        /* 1 means swallow floating windows by default */
static const int showbar            = 1;        /* 0 means no bar */
static const int topbar             = 1;        /* 0 means bottom bar */
static const char *fonts[]          = { "JetBrains Mono:size=9" };
static const char dmenufont[]       = "JetBrains Mono:size=9";

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
	{ "firefoxdeveloperedition", NULL,     NULL,           1 << 8,    0,          0,          -1,        -1 },
	{ "discord",                 NULL,     NULL,           1 << 7,    0,          0,           0,        -1 },
	{ "Spotify",                 NULL,     NULL,           1 << 6,    0,          0,           0,        -1 },
	{ NULL,                      NULL,     "Event Tester", 0,         1,          0,           1,        -1 }, /* xev */
};

/* layout(s) */
static const float mfact     = 0.5;  /* factor of master area size [0.05..0.95] */
static const int nmaster     = 1;    /* number of clients in master area */
static const int resizehints = 0;    /* 1 means respect size hints in tiled resizals */

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

// dbus commands are very long, this helps make them more readable in the config
#define DBUS_SEND_SPOTIFY "dbus-send", "--print-reply", "--dest=org.mpris.MediaPlayer2.spotify", "/org/mpris/MediaPlayer2"
#define VOLUME_SET(x) "pactl", "set-sink-volume", "@DEFAULT_SINK@", x
#define VOLUME_UP(x) VOLUME_SET("+" #x "%")
#define VOLUME_DOWN(x) VOLUME_SET("-" #x "%")

/* commands */
static char dmenumon[2]                 = "0"; /* component of dmenucmd, manipulated in spawn() */
static const char dmenu_highpriority[]  = "spotify,discord,firefox-developer-edition,wireshark,ghidra,google-chrome-stable,zoom";
static const char *dmenucmd[]           = { "dmenu_run", "-m", dmenumon, "-fn", dmenufont, "-nb", col_base00, "-nf", col_base0D, "-sb", col_base08, "-sf", col_base0D, "-hb", col_base0D, "-hf", col_base00, "-hp", dmenu_highpriority, NULL };
static const char *termcmd[]            = { "st", NULL };
static const char *termbigcmd[]         = { "st", "-f", "JetBrains Mono:size=12", NULL };
static const char *brightness_up[]      = { "xbacklight", "-inc", "5", NULL };
static const char *brightness_down[]    = { "xbacklight", "-dec", "5", NULL };
static const char *volume_up[]          = { VOLUME_UP(1), NULL };
static const char *volume_down[]        = { VOLUME_DOWN(1), NULL };
static const char *volume_up_big[]      = { VOLUME_UP(10), NULL };
static const char *volume_down_big[]    = { VOLUME_DOWN(10), NULL };
static const char *volume_mute[]        = { VOLUME_SET("toggle"), NULL };
static const char *spotify_play_pause[] = { DBUS_SEND_SPOTIFY, "org.mpris.MediaPlayer2.Player.PlayPause", NULL };
static const char *spotify_stop[]       = { DBUS_SEND_SPOTIFY, "org.mpris.MediaPlayer2.Player.Stop", NULL };
static const char *spotify_next[]       = { DBUS_SEND_SPOTIFY, "org.mpris.MediaPlayer2.Player.Next", NULL };
static const char *spotify_prev[]       = { DBUS_SEND_SPOTIFY, "org.mpris.MediaPlayer2.Player.Previous", NULL };
static const char *emoji_picker[]       = { "emojipicker", NULL };
static const char *shutdown[]           = { "shutdown", "now", NULL };
static const char *reboot[]             = { "reboot", NULL };
static const char *xkill[]              = { "xkill", NULL };
static const Arg  screenshot            = SHCMD("maim -su | tee ~/Pictures/last_sc.png | xclip -selection clipboard -t image/png");

#include "movestack.c"
static Key keys[] = {
	/* modifier                     key                       function        argument */
	{ MODKEY,                       XK_p,                     spawn,          {.v = dmenucmd } },
	{ MODKEY|ShiftMask,             XK_Return,                spawn,          {.v = termcmd } },
	{ MODKEY,                       XK_Return,                spawn,          {.v = termbigcmd } },
	{ MODKEY,                       XK_Print,                 spawn,          screenshot },
	{ MODKEY|ShiftMask,             XK_s,                     spawn,          screenshot },
	{ MODKEY|ShiftMask,             XK_Delete,                spawn,          {.v = xkill } },
	{ MODKEY,                       XK_b,                     togglebar,      {0} },
	{ MODKEY,                       XK_j,                     focusstack,     {.i = +1 } },
	{ MODKEY,                       XK_k,                     focusstack,     {.i = -1 } },
	{ MODKEY,                       XK_i,                     incnmaster,     {.i = +1 } },
	{ MODKEY,                       XK_d,                     incnmaster,     {.i = -1 } },
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
	// flipped because my monitors are clapped
	{ MODKEY,                       XK_comma,                 focusmon,       {.i = +1 } },
	{ MODKEY,                       XK_period,                focusmon,       {.i = -1 } },
	{ MODKEY|ShiftMask,             XK_comma,                 tagmon,         {.i = -1 } },
	{ MODKEY|ShiftMask,             XK_period,                tagmon,         {.i = +1 } },
	{ MODKEY|ControlMask,           XK_h,                     rotatetags,     {.i = -1 } },
	{ MODKEY|ControlMask,           XK_l,                     rotatetags,     {.i = +1 } },
	TAGKEYS(                      	XK_1,                                     0)
	TAGKEYS(                       	XK_2,                                     1)
	TAGKEYS(                       	XK_3,                                     2)
	TAGKEYS(                       	XK_4,                                     3)
	TAGKEYS(                       	XK_5,                                     4)
	TAGKEYS(                       	XK_6,                                     5)
	TAGKEYS(                       	XK_7,                                     6)
	TAGKEYS(                       	XK_8,                                     7)
	TAGKEYS(                       	XK_9,                                     8)
	{ MODKEY|ShiftMask,             XK_q,                     quit,           {0} },
	{ MODKEY|ControlMask|ShiftMask, XK_q,                     quit,           {1} },
	{ MODKEY|ControlMask|ShiftMask, XK_s,                     spawn,          {.v = shutdown} },
	{ MODKEY|ControlMask|ShiftMask, XK_r,                     spawn,          {.v = reboot} },
	{ MODKEY,                       XK_e,                     spawn,          {.v = emoji_picker} },
	{ 0,                            XF86XK_MonBrightnessUp,   spawn,          {.v = brightness_up } },
	{ 0,                            XF86XK_MonBrightnessDown, spawn,          {.v = brightness_down } },
	{ 0,                            XF86XK_AudioRaiseVolume,  spawn,          {.v = volume_up_big } },
	{ 0,                            XF86XK_AudioLowerVolume,  spawn,          {.v = volume_down_big } },
	{ MODKEY,                       XF86XK_AudioRaiseVolume,  spawn,          {.v = volume_up } },
	{ MODKEY,                       XF86XK_AudioLowerVolume,  spawn,          {.v = volume_down } },
	{ ShiftMask,                    XF86XK_AudioRaiseVolume,  spawn,          {.v = volume_up } },
	{ ShiftMask,                    XF86XK_AudioLowerVolume,  spawn,          {.v = volume_down } },
	{ ControlMask,                  XF86XK_AudioRaiseVolume,  spawn,          {.v = volume_up } },
	{ ControlMask,                  XF86XK_AudioLowerVolume,  spawn,          {.v = volume_down } },
	{ 0,                            XF86XK_AudioMute,         spawn,          {.v = volume_mute } },
	{ 0,                            XF86XK_AudioPlay,         spawn,          {.v = spotify_play_pause } },
	{ 0,                            XF86XK_AudioPause,        spawn,          {.v = spotify_play_pause } },
	{ 0,                            XF86XK_AudioStop,         spawn,          {.v = spotify_stop } },
	{ 0,                            XF86XK_AudioNext,         spawn,          {.v = spotify_next } },
	{ 0,                            XF86XK_AudioPrev,         spawn,          {.v = spotify_prev } },
	{ MODKEY | ShiftMask,           XK_bracketleft,           spawn,          {.v = spotify_prev } },
	{ MODKEY | ShiftMask,           XK_bracketright,          spawn,          {.v = spotify_next } },
	{ MODKEY | ShiftMask,           XK_p,                     spawn,          {.v = spotify_play_pause } },
	// mirror windows bindings
	{ WINKEY|ControlMask,           XK_Left,                  rotatetags,     {.i = -1 } },
	{ WINKEY|ControlMask,           XK_Right,                 rotatetags,     {.i = +1 } },
	{ WINKEY|ShiftMask,             XK_s,                     spawn,          screenshot },
	// map both cos why not lol
	{ MODKEY|ShiftMask,             XK_s,                     spawn,          screenshot },
};

/* button definitions */
/* click can be ClkTagBar, ClkLtSymbol, ClkStatusText, ClkWinTitle, ClkClientWin, or ClkRootWin */
static Button buttons[] = {
	/* click                event mask      button          function        argument */
	{ ClkLtSymbol,          0,              Button1,        setlayout,      {0} },
	{ ClkLtSymbol,          0,              Button3,        setlayout,      {.v = &layouts[2]} },
	{ ClkWinTitle,          0,              Button2,        zoom,           {0} },
	{ ClkStatusText,        0,              Button2,        spawn,          {.v = termcmd } },
	{ ClkClientWin,         MODKEY,         Button1,        movemouse,      {0} },
	{ ClkClientWin,         MODKEY,         Button2,        togglefloating, {0} },
	{ ClkClientWin,         MODKEY,         Button3,        resizemouse,    {0} },
	{ ClkTagBar,            0,              Button1,        view,           {0} },
	{ ClkTagBar,            0,              Button3,        toggleview,     {0} },
	{ ClkTagBar,            MODKEY,         Button1,        tag,            {0} },
	{ ClkTagBar,            MODKEY,         Button3,        toggletag,      {0} },
};

