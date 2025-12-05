//
// Copyright(C) 2024 Roman Fomin
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

#include "st_widgets.h"

#include <math.h>
#include <string.h>

#include "d_deh.h"
#include "d_event.h"
#include "d_player.h"
#include "doomdef.h"
#include "doomkeys.h"
#include "doomstat.h"
#include "doomtype.h"
#include "dstrings.h"
#include "g_umapinfo.h"
#include "hu_command.h"
#include "hu_coordinates.h"
#include "hu_obituary.h"
#include "i_timer.h"
#include "i_video.h"
#include "m_array.h"
#include "m_config.h"
#include "m_input.h"
#include "m_misc.h"
#include "mn_menu.h"
#include "p_mobj.h"
#include "p_spec.h"
#include "r_main.h"
#include "r_voxel.h"
#include "s_sound.h"
#include "sounds.h"
#include "st_sbardef.h"
#include "st_stuff.h"
#include "v_video.h"

// [Nugget]
#include "m_nughud.h"
#include "st_stuff.h"
#include "z_zone.h"

// [Nugget] /=================================================================

// CVARs ---------------------------------------------------------------------

enum {
  SHOWSTATS_KILLS,
  SHOWSTATS_ITEMS,
  SHOWSTATS_SECRETS,

  NUMSHOWSTATS
};

static boolean hud_stats_show[NUMSHOWSTATS];
static boolean hud_stats_show_map[NUMSHOWSTATS];

boolean hud_time[NUMTIMERS];

static int hud_power_timers;
static boolean hud_power_timers_notime;

static boolean hud_allow_icons;

static int hudcolor_time_scale;
static int hudcolor_total_time;
static int hudcolor_time;
static int hudcolor_event_timer;
static int hudcolor_kills;
static int hudcolor_items;
static int hudcolor_secrets;
static int hudcolor_ms_incomp;
static int hudcolor_ms_comp;

boolean announce_milestones;
boolean show_save_messages;
static boolean message_fadeout;
static boolean message_flash;
static int hud_msg_duration;
static int hud_chat_duration;
static int hud_msg_lines;
static int hud_msg_total_lines;
static boolean hud_msg_scrollup;
static boolean hud_msg_group;

boolean sp_chat;

// ---------------------------------------------------------------------------

boolean ST_MessageFadeoutOn(void)
{
    return message_fadeout;
}

int ST_GetNumMessageLines(void)
{
    return MAX(hud_msg_total_lines, hud_msg_lines);
}

void FadeOutLine(widgetline_t *const line, const int duration_left)
{
    if (message_fadeout && 0 <= duration_left && duration_left < 9)
    {
        line->tran_pct = (duration_left + 1) * 10;
    }
    else { line->tran_pct = 0; }
}

// [Nugget] =================================================================/

boolean       show_messages;
boolean       show_toggle_messages;
boolean       show_pickup_messages;

secretmessage_t hud_secret_message; // "A secret is revealed!" message
widgetstate_t hud_level_stats;
widgetstate_t hud_level_time;
boolean       hud_time_use;
widgetstate_t hud_player_coords;
widgetstate_t hud_widget_font;

static boolean hud_map_announce;
static boolean message_colorized;

//jff 2/16/98 hud supported automap colors added
int hudcolor_titl;  // color range of automap level title
int hudcolor_xyco;  // color range of new coords on automap

boolean chat_on;

// [Nugget]
boolean ST_GetChatOn(void)
{
  return chat_on;
}

void ST_ClearLines(sbe_widget_t *widget)
{
    array_clear(widget->lines);
}

void ST_AddLine(sbe_widget_t *widget, const char *string)
{
    widgetline_t line = { .string = string };
    array_push(widget->lines, line);
}

static void SetLine(sbe_widget_t *widget, const char *string)
{
    array_clear(widget->lines);
    widgetline_t line = { .string = string };
    array_push(widget->lines, line);
}

static char message_string[HU_MAXLINELENGTH];

static boolean message_review;

// [Nugget] Message list /----------------------------------------------------

static const int MAX_COPIES = 99999;

typedef struct linkedmessage_s
{
    struct linkedmessage_s *prev, *next;
    char string[HU_MAXLINELENGTH + 8]; // Extra space for " [#]", where # is MAX_COPIES
    size_t orig_length;
    int duration_left;
    int flash_duration_left; // Message flash

    // Messages from other players cannot be overwritten by normal messages
    // until their duration runs out
    boolean is_chat_msg;
} linkedmessage_t;

static linkedmessage_t *message_list_head = NULL, *message_list_tail = NULL;

static int num_messages = 0;
static int message_review_duration_left = 0;

static void AddMessage(char *const string, int duration, const boolean is_chat_msg)
{
    static int num_copies = 0;

    if (hud_msg_group && message_list_tail && num_copies < MAX_COPIES
        && strlen(string) == message_list_tail->orig_length
        && !strncmp(string, message_list_tail->string, message_list_tail->orig_length))
    {
        num_copies++;

        M_snprintf(
            message_list_tail->string + message_list_tail->orig_length,
            sizeof(message_list_tail->string) - message_list_tail->orig_length,
            " [%i]", num_copies
        );
    }
    else {
        num_messages++;
        num_copies = 1;

        if (message_list_tail && num_messages > ST_GetNumMessageLines())
        {
            num_messages--;

            linkedmessage_t *m = message_list_head;

            // Look for a non-chat message to remove
            while (m->is_chat_msg && m->next) { m = m->next; }

            if (m->is_chat_msg)
            {
                // All messages in the list are chat messages

                // Is the new one a chat message too?
                if (is_chat_msg)
                {
                    // Yes; advance the list normally
                    m = message_list_head;
                }
                else {
                    // No; omit it
                    goto clear_string;
                }
            }

            linkedmessage_t *const prev = m->prev,
                            *const next = m->next;

            Z_Free(m);

            if (prev) { prev->next = next; }
            else      { message_list_head = next; }

            if (next) { next->prev = prev; }
            else      { message_list_tail = prev; }
        }

        if (message_list_tail)
        {
            message_list_tail->next = Z_Malloc(sizeof(linkedmessage_t), PU_STATIC, NULL);

            message_list_tail->next->prev = message_list_tail;
            message_list_tail = message_list_tail->next;
        }
        else {
            message_list_head =
            message_list_tail = Z_Malloc(sizeof(linkedmessage_t), PU_STATIC, NULL);

            message_list_tail->prev = NULL;
        }

        message_list_tail->next = NULL;

        M_StringCopy(message_list_tail->string, string, sizeof(message_list_tail->string));
        message_list_tail->orig_length = strlen(message_list_tail->string);
    }

    if (is_chat_msg)
    {
        message_list_tail->duration_left = hud_chat_duration ? hud_chat_duration : duration;
        message_list_tail->is_chat_msg = true;
    }
    else {
        message_list_tail->duration_left = hud_msg_duration ? hud_msg_duration : duration;
        message_list_tail->is_chat_msg = false;
    }

    // Message flash (actually 4, as it will be decremented right after this)
    message_list_tail->flash_duration_left = message_flash ? 5 : 0;

clear_string:

    string[0] = '\0';
}

void ST_ClearMessages(void)
{
    linkedmessage_t *m = message_list_head;

    while (m)
    {
        linkedmessage_t *const deletee = m;
        m = m->next;
        Z_Free(deletee);
    }

    message_list_head = message_list_tail = NULL;
    num_messages = 0;
    message_review_duration_left = 0;
}

void ST_HideMessages(void)
{
    linkedmessage_t *m = message_list_head;

    while (m)
    {
        m->duration_left = 0;
        m = m->next;
    }

    message_review_duration_left = 0;
}

// [Nugget] -----------------------------------------------------------------/

static void UpdateMessage(sbe_widget_t *widget, player_t *player)
{
    // [Nugget] Rewritten to make use of list system

    ST_ClearLines(widget);

    // Handle setting changes
    while (num_messages > ST_GetNumMessageLines())
    {
        num_messages--;

        message_list_head = message_list_head->next;
        Z_Free(message_list_head->prev);

        message_list_head->prev = NULL;
    }

    static boolean messages_enabled = true;

    if (messages_enabled)
    {
        if (message_string[0])
        {
            AddMessage(message_string, widget->duration, true);
        }
        else if (player->message && player->message[0])
        {
            AddMessage(player->message, widget->duration, false);
        }
        else if (message_review)
        {
            message_review = false;
            message_review_duration_left = hud_msg_duration ? hud_msg_duration : widget->duration;
        }
    }

    if (messages_enabled != show_messages)
    {
        messages_enabled = show_messages;
    }

    // If true, show full list, otherwise maybe omit some messages
    if (message_review_duration_left > 0)
    {
        message_review_duration_left--;
    }

    linkedmessage_t *m = hud_msg_scrollup ? message_list_head : message_list_tail;

    for (int index = hud_msg_scrollup ? 0 : num_messages-1;  m;)
    {
        if ((m->duration_left > 0 && index >= num_messages - hud_msg_lines)
            || message_review_duration_left > 0)
        {
            if (m->duration_left > 0) { m->duration_left--; }

            ST_AddLine(widget, m->string);

            widgetline_t *const line = &widget->lines[array_size(widget->lines) - 1];

            // Message fadeout -----------------------------------------------

            const int fadeout_time = (index < num_messages - hud_msg_lines)
                                   ? message_review_duration_left
                                   : MAX(m->duration_left, message_review_duration_left);

            FadeOutLine(line, fadeout_time);

            // Message flash -------------------------------------------------

            if (m->flash_duration_left > 0) { m->flash_duration_left--; }

            line->flash = !!m->flash_duration_left;
        }
        else {
            m->duration_left = 0;
            m->is_chat_msg = false; // Allow overwriting
            m->flash_duration_left = 0;
        }

        if (hud_msg_scrollup)
        {
            m = m->next;
            index++;
        }
        else {
            m = m->prev;
            index--;
        }
    }
}

static char announce_string[HU_MAXLINELENGTH], author_string[HU_MAXLINELENGTH];

static void UpdateAnnounceMessage(sbe_widget_t *widget, player_t *player)
{
    static enum
    {
        announce_none,
        announce_map,
        announce_secret
    } state = announce_none;

    ST_ClearLines(widget);

    if ((state == announce_secret && !hud_secret_message && !announce_milestones) // [Nugget]
        || (state == announce_map && !hud_map_announce))
    {
        return;
    }

    static char string[HU_MAXLINELENGTH];

    if (announce_string[0])
    {
        state = announce_map;
        widget->duration_left = widget->duration;
        M_StringCopy(string, announce_string, sizeof(string));
        announce_string[0] = '\0';
    }
    else if (player->secretmessage)
    {
        author_string[0] = '\0';
        state = announce_secret;
        widget->duration_left = widget->duration;
        M_snprintf(string, sizeof(string), GOLD_S "%s" ORIG_S,
            player->secretmessage);
        player->secretmessage = NULL;
    }

    if (widget->duration_left > 0)
    {
        ST_AddLine(widget, string);
        if (author_string[0])
        {
            ST_AddLine(widget, author_string);
        }
        --widget->duration_left;

        // [Nugget] Message fadeout
        FadeOutLine(&widget->lines[array_size(widget->lines) - 1], widget->duration_left);
    }
    else
    {
        state = announce_none;
    }
}

// key tables
// jff 5/10/98 french support removed, 
// as it was not being used and couldn't be easily tested
//

static const char shiftxform[] =
{
    0,
    1, 2, 3, 4, 5, 6, 7, 8, 9, 10,
    11, 12, 13, 14, 15, 16, 17, 18, 19, 20,
    21, 22, 23, 24, 25, 26, 27, 28, 29, 30,
    31,
    ' ', '!', '"', '#', '$', '%', '&',
    '"', // shift-'
    '(', ')', '*', '+',
    '<', // shift-,
    '_', // shift--
    '>', // shift-.
    '?', // shift-/
    ')', // shift-0
    '!', // shift-1
    '@', // shift-2
    '#', // shift-3
    '$', // shift-4
    '%', // shift-5
    '^', // shift-6
    '&', // shift-7
    '*', // shift-8
    '(', // shift-9
    ':',
    ':', // shift-;
    '<',
    '+', // shift-=
    '>', '?', '@',
    'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N',
    'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',
    '[', // shift-[
    '!', // shift-backslash - OH MY GOD DOES WATCOM SUCK
    ']', // shift-]
    '"', '_',
    '\'', // shift-`
    'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N',
    'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',
    '{', '|', '}', '~', 127
};

typedef struct
{
    char string[HU_MAXLINELENGTH];
    int pos;
} chatline_t;

static chatline_t lines[MAXPLAYERS];

static void ClearChatLine(chatline_t *line)
{
    line->pos = 0;
    line->string[0] = '\0';
}

static boolean AddKeyToChatLine(chatline_t *line, char ch)
{
    if (ch >= ' ' && ch <= '_')
    {
        if (line->pos == HU_MAXLINELENGTH - 1)
        {
            return false;
        }
        line->string[line->pos++] = ch;
        line->string[line->pos] = '\0';
    }
    else if (ch == KEY_BACKSPACE) // phares
    {
        if (line->pos == 0)
        {
            return false;
        }
        else
        {
            line->string[--line->pos] = '\0';
        }
    }
    else if (ch != KEY_ENTER) // phares
    {
        return false; // did not eat key
    }

    return true; // ate the key
}

#define HU_BROADCAST 5

char **player_names[] =
{
    &s_HUSTR_PLRGREEN,
    &s_HUSTR_PLRINDIGO,
    &s_HUSTR_PLRBROWN,
    &s_HUSTR_PLRRED
};

void ST_UpdateChatMessage(void)
{
    static char chat_dest[MAXPLAYERS];

    for (int p = 0; p < MAXPLAYERS; p++)
    {
        if (!playeringame[p])
        {
            continue;
        }

        char ch = players[p].cmd.chatchar;
        if (p != consoleplayer && ch)
        {
            if (ch <= HU_BROADCAST)
            {
                chat_dest[p] = ch;
            }
            else
            {
                if (ch >= 'a' && ch <= 'z')
                {
                    ch = (char)shiftxform[(unsigned char)ch];
                }

                if (AddKeyToChatLine(&lines[p], ch) && ch == KEY_ENTER)
                {
                    if (lines[p].pos && (chat_dest[p] == consoleplayer + 1
                                         || chat_dest[p] == HU_BROADCAST))
                    {
                        M_snprintf(message_string, sizeof(message_string),
                            "%s%s", *player_names[p], lines[p].string);

                        S_StartSoundPitch(0,
                                          gamemode == commercial ? sfx_radio
                                                                 : sfx_tink,
                                          PITCH_NONE);
                    }
                    ClearChatLine(&lines[p]);
                }
            }
            players[p].cmd.chatchar = 0;
        }
    }
}

static const char *chat_macros[] = {
    HUSTR_CHATMACRO0, HUSTR_CHATMACRO1, HUSTR_CHATMACRO2, HUSTR_CHATMACRO3,
    HUSTR_CHATMACRO4, HUSTR_CHATMACRO5, HUSTR_CHATMACRO6, HUSTR_CHATMACRO7,
    HUSTR_CHATMACRO8, HUSTR_CHATMACRO9
};

#define QUEUESIZE   128

static char chatchars[QUEUESIZE];
static int  head = 0;
static int  tail = 0;

//
// QueueChatChar()
//
// Add an incoming character to the circular chat queue
//
// Passed the character to queue, returns nothing
//

static void QueueChatChar(char ch)
{
    if (((head + 1) & (QUEUESIZE - 1)) == tail)
    {
        displaymsg("%s", HUSTR_MSGU);
    }
    else
    {
        chatchars[head++] = ch;
        head &= QUEUESIZE - 1;
    }
}

//
// ST_DequeueChatChar()
//
// Remove the earliest added character from the circular chat queue
//
// Passed nothing, returns the character dequeued
//

char ST_DequeueChatChar(void)
{
    char ch;

    if (head != tail)
    {
        ch = chatchars[tail++];
        tail &= QUEUESIZE - 1;
    }
    else
    {
        ch = 0;
    }

    return ch;
}

static chatline_t chatline;

boolean ST_MessagesResponder(event_t *ev)
{
    if (ev->type == ev_text)
    {
        return false;
    }

    static char lastmessage[HU_MAXLINELENGTH + 1];

    boolean eatkey = false;
    static boolean shiftdown = false;
    static boolean altdown = false;
    int ch;
    int numplayers;

    static int num_nobrainers = 0;

    ch = (ev->type == ev_keydown) ? ev->data1.i : 0;

    numplayers = 0;
    for (int p = 0; p < MAXPLAYERS; p++)
    {
        numplayers += playeringame[p];
    }

    if (ev->data1.i == KEY_RSHIFT)
    {
        shiftdown = ev->type == ev_keydown;
        return false;
    }

    if (ev->data1.i == KEY_RALT)
    {
        altdown = ev->type == ev_keydown;
        return false;
    }

    if (M_InputActivated(input_chat_backspace))
    {
        ch = KEY_BACKSPACE;
    }

    if (!chat_on)
    {
        if (M_InputActivated(input_chat_enter)) // phares
        {
            //jff 2/26/98 toggle list of messages
            message_review = true;
            eatkey = true;
        }
        else if (demoplayback) // killough 10/02/98: no chat if demo playback
        {
            eatkey = false;
        }
        else if ((netgame || sp_chat) // [Nugget]
                 && M_InputActivated(input_chat))
        {
            eatkey = chat_on = true;
            ClearChatLine(&chatline);
            QueueChatChar(HU_BROADCAST);
        }
        else if (netgame && numplayers > 2) // killough 11/98: simplify
        {
            for (int p = 0; p < MAXPLAYERS; p++)
            {
                if (M_InputActivated(input_chat_dest0 + p))
                {
                    if (p == consoleplayer)
                    {
                        displaymsg("%s",
                                   ++num_nobrainers < 3 ? HUSTR_TALKTOSELF1
                                   : num_nobrainers < 6 ? HUSTR_TALKTOSELF2
                                   : num_nobrainers < 9 ? HUSTR_TALKTOSELF3
                                   : num_nobrainers < 32
                                       ? HUSTR_TALKTOSELF4
                                       : HUSTR_TALKTOSELF5);
                    }
                    else if (playeringame[p])
                    {
                        eatkey = chat_on = true;
                        ClearChatLine(&chatline);
                        QueueChatChar((char)(p + 1));
                        break;
                    }
                }
            }
        }
    } // jff 2/26/98 no chat functions if message review is displayed
    else
    {
        if (M_InputActivated(input_chat_enter))
        {
            ch = KEY_ENTER;
        }

        // send a macro
        if (altdown)
        {
            ch = ch - '0';
            if (ch < 0 || ch > 9)
            {
                return false;
            }
            const char *macromessage = chat_macros[ch];

            // kill last message with a '\n'
            QueueChatChar(KEY_ENTER); // DEBUG!!!                // phares

            // send the macro message
            while (*macromessage)
            {
                QueueChatChar(*macromessage++);
            }
            QueueChatChar(KEY_ENTER); // phares

            // leave chat mode and notify that it was sent
            chat_on = false;
            M_StringCopy(lastmessage, chat_macros[ch], sizeof(lastmessage));
            displaymsg("%s", lastmessage);
            eatkey = true;
        }
        else
        {
            if (shiftdown || (ch >= 'a' && ch <= 'z'))
            {
                ch = shiftxform[ch];
            }
            eatkey = AddKeyToChatLine(&chatline, ch);
            if (eatkey)
            {
                QueueChatChar(ch);
            }

            if (ch == KEY_ENTER) // phares
            {
                chat_on = false;
                if (chatline.pos)
                {
                    M_StringCopy(lastmessage, chatline.string,
                                 sizeof(lastmessage));
                    displaymsg("%s", lastmessage);
                }
            }
            else if (ch == KEY_ESCAPE) // phares
            {
                chat_on = false;
            }
        }
    }
    return eatkey;
}

static void UpdateChat(sbe_widget_t *widget)
{
    ST_ClearLines(widget);

    static char string[HU_MAXLINELENGTH + 1];

    string[0] = '\0';

    if (chat_on)
    {
        M_StringCopy(string, chatline.string, sizeof(string));

        if (leveltime & 16)
        {
            M_StringConcat(string, "_", sizeof(string));
        }
        ST_AddLine(widget, string);
    }
}

static boolean IsVanillaMap(int e, int m)
{
    if (gamemode == commercial)
    {
        return (e == 1 && m > 0 && m <= 32);
    }
    else
    {
        return (e > 0 && e <= 4 && m > 0 && m <= 9);
    }
}

#define HU_TITLE  (*mapnames[(gameepisode - 1) * 9 + gamemap - 1])
#define HU_TITLE2 (*mapnames2[gamemap - 1])
#define HU_TITLEP (*mapnamesp[gamemap - 1])
#define HU_TITLET (*mapnamest[gamemap - 1])

static char title_string[HU_MAXLINELENGTH];

void ST_ResetTitle(void)
{
    char string[120];
    string[0] = '\0';

    char *s;

    if (gamemapinfo && gamemapinfo->levelname)
    {
        if (gamemapinfo->label)
        {
            s = gamemapinfo->label;
        }
        else
        {
            s = gamemapinfo->mapname;
        }

        if (!(gamemapinfo->flags & MapInfo_LabelClear))
        {
            M_snprintf(string, sizeof(string), "%s: ", s);
        }

        s = gamemapinfo->levelname;
    }
    else if (gamestate == GS_LEVEL)
    {
        if (IsVanillaMap(gameepisode, gamemap))
        {
            s = (gamemode != commercial)     ? HU_TITLE
                : (gamemission == pack_tnt)  ? HU_TITLET
                : (gamemission == pack_plut) ? HU_TITLEP
                                             : HU_TITLE2;
        }
        // WADs like pl2.wad have a MAP33, and rely on the layout in the
        // Vanilla executable, where it is possible to overflow the end of one
        // array into the next.
        else if (gamemode == commercial && gamemap >= 33 && gamemap <= 35)
        {
            s = (gamemission == doom2)       ? (*mapnamesp[gamemap - 33])
                : (gamemission == pack_plut) ? (*mapnamest[gamemap - 33])
                                             : "";
        }
        else
        {
            // initialize the map title widget with the generic map lump name
            s = MapName(gameepisode, gamemap);
        }
    }
    else
    {
        s = "";
    }

    char *n;

    // [FG] cap at line break
    if ((n = strchr(s, '\n')))
    {
        *n = '\0';
    }

    M_StringConcat(string, s, sizeof(string));

    title_string[0] = '\0';
    M_snprintf(title_string, sizeof(title_string), "\x1b%c%s" ORIG_S,
               '0' + hudcolor_titl, string);

    announce_string[0] = '\0';
    author_string[0] = '\0';
    if (hud_map_announce && leveltime == 0)
    {
        if (gamemapinfo && gamemapinfo->author)
        {
            M_snprintf(announce_string, sizeof(announce_string), "%s by %s",
                       string, gamemapinfo->author);
            if (MN_StringWidth(announce_string) > SCREENWIDTH) 
            {
                M_StringCopy(announce_string, string, sizeof(announce_string));
                M_snprintf(author_string, sizeof(author_string), "by %s",
                           gamemapinfo->author);
            }
        }
        else
        {
            M_StringCopy(announce_string, string, sizeof(announce_string));
        }
    }
}

static void UpdateTitle(sbe_widget_t *widget)
{
    SetLine(widget, title_string);
}

static boolean WidgetEnabled(widgetstate_t state)
{
    if (automapactive == AM_FULL && !(state & HUD_WIDGET_AUTOMAP))
    {
        return false;
    }
    else if (automapactive != AM_FULL && !(state & HUD_WIDGET_HUD))
    {
        return false;
    }
    return true;
}

static void ForceDoomFont(sbe_widget_t *widget)
{
    if (WidgetEnabled(hud_widget_font))
    {
        widget->font = stcfnt;
    }
    else
    {
        widget->font = widget->default_font;
    }
}

static void UpdateCoord(sbe_widget_t *widget, player_t *player)
{
    ST_ClearLines(widget);

    if (strictmode)
    {
        return;
    }

    if (hud_player_coords == HUD_WIDGET_ADVANCED)
    {
        HU_BuildCoordinatesEx(widget, player->mo);
        return;
    }

    if (!WidgetEnabled(hud_player_coords))
    {
        return;
    }

    ForceDoomFont(widget);

    fixed_t x, y, z; // killough 10/98:
    void AM_Coordinates(const mobj_t *, fixed_t *, fixed_t *, fixed_t *);

    // killough 10/98: allow coordinates to display non-following pointer
    AM_Coordinates(player->mo, &x, &y, &z);

    if (!widget->vertical)
    {
        static char string[80];
        // jff 2/16/98 output new coord display
        M_snprintf(string, sizeof(string),
                   "\x1b%cX " GRAY_S "%d \x1b%cY " GRAY_S "%d \x1b%cZ " GRAY_S "%d",
                   '0' + hudcolor_xyco, x >> FRACBITS, '0' + hudcolor_xyco,
                   y >> FRACBITS, '0' + hudcolor_xyco, z >> FRACBITS);
        ST_AddLine(widget, string);
    }
    else
    {
        static char string1[16];
        M_snprintf(string1, sizeof(string1), "\x1b%cX " GRAY_S "%d",
                   '0' + hudcolor_xyco, x >> FRACBITS);
        ST_AddLine(widget, string1);
        static char string2[16];
        M_snprintf(string2, sizeof(string2), "\x1b%cY " GRAY_S "%d",
                   '0' + hudcolor_xyco, y >> FRACBITS);
        ST_AddLine(widget, string2);
        static char string3[16];
        M_snprintf(string3, sizeof(string3), "\x1b%cZ " GRAY_S "%d",
                   '0' + hudcolor_xyco, z >> FRACBITS);
        ST_AddLine(widget, string3);
    }
}

typedef enum
{
    STATSFORMAT_MATCHHUD, // [Nugget]
    STATSFORMAT_RATIO,
    STATSFORMAT_BOOLEAN,
    STATSFORMAT_PERCENT,
    STATSFORMAT_REMAINING,
    STATSFORMAT_COUNT,

    NUM_STATSFORMATS // [Nugget]
} statsformat_t;

static statsformat_t hud_stats_format;
static statsformat_t hud_stats_format_map; // [Nugget]

static void StatsFormatFunc_Ratio(char *buffer, size_t size, const int count,
                                  const int total)
{
    M_snprintf(buffer, size, "%d/%d", count, total);
}

static void StatsFormatFunc_Boolean(char *buffer, size_t size, const int count,
                                    const int total)
{
    M_snprintf(buffer, size, "%s", (count >= total) ? "YES" : "NO");
}

static void StatsFormatFunc_Percent(char *buffer, size_t size, const int count,
                                    const int total)
{
    M_snprintf(buffer, size, "%d%%", !total ? 100 : count * 100 / total);
}

static void StatsFormatFunc_Remaining(char *buffer, size_t size,
                                      const int count, const int total)
{
    M_snprintf(buffer, size, "%d", total - count);
}

static void StatsFormatFunc_Count(char *buffer, size_t size, const int count,
                                  const int total)
{
    M_snprintf(buffer, size, "%d", count);
}

typedef void (*statsformatfunc_t)(char *buffer, size_t size, const int count,
                                  const int total);

static const statsformatfunc_t StatsFormatFuncs[] = {
    StatsFormatFunc_Ratio,     StatsFormatFunc_Boolean, StatsFormatFunc_Percent,
    StatsFormatFunc_Remaining, StatsFormatFunc_Count,
};

static void UpdateMonSec(sbe_widget_t *widget)
{
    ST_ClearLines(widget);

    if (!WidgetEnabled(hud_level_stats))
    {
        return;
    }

    // [Nugget] /-------------------------------------------------------------

    const boolean *const showstats = (automapactive == AM_FULL) ? hud_stats_show_map : hud_stats_show;

    if (!(showstats[SHOWSTATS_KILLS] || showstats[SHOWSTATS_ITEMS] || showstats[SHOWSTATS_SECRETS]))
    { return; }

    // [Nugget] -------------------------------------------------------------/

    ForceDoomFont(widget);

    int fullkillcount = 0;
    int fullitemcount = 0;
    int fullsecretcount = 0;
    int kill_percent_count = 0;

    for (int i = 0; i < MAXPLAYERS; ++i)
    {
        if (playeringame[i])
        {
            fullkillcount += players[i].killcount - players[i].maxkilldiscount;
            fullitemcount += players[i].itemcount;
            fullsecretcount += players[i].secretcount;
            kill_percent_count += players[i].killcount;
        }
    }

    if (respawnmonsters)
    {
        fullkillcount = kill_percent_count;
        max_kill_requirement = totalkills;
    }

    // [Nugget] Customizable Stats colors

    char killlabelcolor, itemlabelcolor, secretlabelcolor;

    int killcolor = (fullkillcount >= max_kill_requirement) ? '0' + hudcolor_ms_comp
                                                            : '0' + hudcolor_ms_incomp;
    int secretcolor =
        (fullsecretcount >= totalsecret) ? '0' + hudcolor_ms_comp : '0' + hudcolor_ms_incomp;
    int itemcolor =
        (fullitemcount >= totalitems) ? '0' + hudcolor_ms_comp : '0' + hudcolor_ms_incomp;

    // [Nugget] HUD icons /---------------------------------------------------

    char killlabel, itemlabel, secretlabel;

    #define USE_ICON(_labelvar_, _label_, _labelcolorvar_, _labelcolor_, _icon_) \
        if (hud_allow_icons && ST_IconAvailable(_icon_))                         \
        {                                                                        \
            _labelvar_ = (char) (HU_FONTEND + 1 + _icon_);                       \
            _labelcolorvar_ = '0' + CR_NONE;                                     \
        }                                                                        \
        else {                                                                   \
            _labelvar_ = _label_;                                                \
            _labelcolorvar_ = '0' + _labelcolor_;                                \
        }

    USE_ICON(killlabel,   'K', killlabelcolor,   hudcolor_kills,   0);
    USE_ICON(itemlabel,   'I', itemlabelcolor,   hudcolor_items,   1);
    USE_ICON(secretlabel, 'S', secretlabelcolor, hudcolor_secrets, 2);

    #undef USE_ICON

    // [Nugget] -------------------------------------------------------------/

    char kill_str[16], item_str[16], secret_str[16];

    statsformatfunc_t StatsFormatFunc = StatsFormatFuncs[
        // [Nugget]
        ((automapactive == AM_FULL && hud_stats_format_map)
         ? hud_stats_format_map : hud_stats_format) - 1
    ];

    StatsFormatFunc(kill_str, sizeof(kill_str), fullkillcount, max_kill_requirement);
    StatsFormatFunc(item_str, sizeof(item_str), fullitemcount, totalitems);
    StatsFormatFunc(secret_str, sizeof(secret_str), fullsecretcount, totalsecret);

    static char kill_str2[24], item_str2[24], secret_str2[24];

    memset(kill_str2,   0, sizeof(kill_str2));
    memset(item_str2,   0, sizeof(item_str2));
    memset(secret_str2, 0, sizeof(secret_str2));

    // [Nugget] Make each stat optional

    if (showstats[SHOWSTATS_KILLS])
    {
        M_snprintf(kill_str2, sizeof(kill_str2),
          "\x1b%c%c \x1b%c%s",
          killlabelcolor, killlabel, killcolor, kill_str);
    }

    if (showstats[SHOWSTATS_ITEMS])
    {
        M_snprintf(item_str2, sizeof(item_str2),
          "\x1b%c%c \x1b%c%s",
          itemlabelcolor, itemlabel, itemcolor, item_str);
    }

    if (showstats[SHOWSTATS_SECRETS])
    {
        M_snprintf(secret_str2, sizeof(secret_str2),
          "\x1b%c%c \x1b%c%s",
          secretlabelcolor, secretlabel, secretcolor, secret_str);
    }

    if (!widget->vertical)
    {
        static char string[120];
        int offset = 0;

        if (kill_str2[0])
        {
            offset += M_snprintf(
              string + offset, sizeof(string) - offset, "%s%s", kill_str2,
              (item_str2[0] || secret_str2[0]) ? " " : ""
            );
        }

        if (item_str2[0])
        {
            offset += M_snprintf(
              string + offset, sizeof(string) - offset, "%s%s", item_str2,
              secret_str2[0] ? " " : ""
            );
        }

        if (secret_str2[0])
        {
            offset += M_snprintf(
              string + offset, sizeof(string) - offset, "%s", secret_str2
            );
        }
  
        ST_AddLine(widget, string);
    }
    else
    {
        if (kill_str2[0])   { ST_AddLine(widget, kill_str2); }
        if (item_str2[0])   { ST_AddLine(widget, item_str2); }
        if (secret_str2[0]) { ST_AddLine(widget, secret_str2); }
    }
}

static void UpdateDM(sbe_widget_t *widget)
{
    ST_ClearLines(widget);

    if (!WidgetEnabled(hud_level_stats))
    {
        return;
    }

    ForceDoomFont(widget);

    static char string[120];

    const int cr_blue = (widget->font == stcfnt) ? CR_BLUE2 : CR_BLUE1;

    int offset = 0;

    for (int i = 0; i < MAXPLAYERS; ++i)
    {
        int result = 0, others = 0;

        if (!playeringame[i])
        {
            continue;
        }

        for (int p = 0; p < MAXPLAYERS; ++p)
        {
            if (!playeringame[p])
            {
                continue;
            }

            if (i != p)
            {
                result += players[i].frags[p];
                others -= players[p].frags[i];
            }
            else
            {
                result -= players[i].frags[p];
            }
        }

        offset += M_snprintf(string + offset, sizeof(string) - offset,
                             "\x1b%c%d/%d ", (i == displayplayer) ?
                             '0' + cr_blue : '0' + CR_GRAY, result, others);
    }

    ST_AddLine(widget, string);
}

static void UpdateStTime(sbe_widget_t *widget, player_t *player)
{
    ST_ClearLines(widget);

    if (!WidgetEnabled(hud_level_time) && !player->btuse_tics)
    {
        return;
    }

    ForceDoomFont(widget);

    static char string[80];

    int offset = 0;

    // [Nugget] Colors

    if (WidgetEnabled(hud_level_time))
    {
        if (time_scale != 100)
        {
            offset +=
                M_snprintf(string, sizeof(string), "\x1b%c%d%% ",
                           '0'+hudcolor_time_scale, time_scale);
        }

        if (levelTimer == true)
        {
            const int time = levelTimeCount / TICRATE;

            offset += M_snprintf(string + offset, sizeof(string) - offset,
                                 BROWN_S "%d:%02d ", time / 60, time % 60);
        }
        else if (totalleveltimes)
        {
            const int time = (totalleveltimes + leveltime) / TICRATE;

            offset += M_snprintf(string + offset, sizeof(string) - offset,
                                 "\x1b%c%d:%02d ", '0'+hudcolor_total_time, time / 60, time % 60);
        }
    }

    if (player->btuse_tics)
    {
        const int type = player->eventtype;

        // [Nugget] Support other events
        M_snprintf(string + offset, sizeof(string) - offset,
                   "\x1b%c%c %d:%05.2f\t",
                   '0'+hudcolor_event_timer,
                   type == TIMER_KEYPICKUP ? 'K' : type == TIMER_TELEPORT ? 'T' : 'U',
                   player->btuse / TICRATE / 60, 
                   (float)(player->btuse % (60 * TICRATE)) / TICRATE);
    }
    else
    {
        M_snprintf(string + offset, sizeof(string) - offset,
                   "\x1b%c%d:%05.2f\t", '0'+hudcolor_time, leveltime / TICRATE / 60,
                   (float)(leveltime % (60 * TICRATE)) / TICRATE);
    }

    ST_AddLine(widget, string);
}

static void UpdateFPS(sbe_widget_t *widget, player_t *player)
{
    ST_ClearLines(widget);

    if (!(player->cheats & CF_SHOWFPS))
    {
        return;
    }

    ForceDoomFont(widget);

    static char string[20];
    M_snprintf(string, sizeof(string), GRAY_S "%d " GREEN_S "FPS", fps);
    ST_AddLine(widget, string);
}

static void UpdateRate(sbe_widget_t *widget, player_t *player)
{
    ST_ClearLines(widget);

    if (!(player->cheats & CF_RENDERSTATS))
    {
        return;
    }

    static char line1[80];
    M_snprintf(line1, sizeof(line1),
               GRAY_S "Sprites %4d Segs %4d Visplanes %4d   " GREEN_S
                      "FPS %3d %dx%d",
               rendered_vissprites, rendered_segs, rendered_visplanes,
               fps, video.width, video.height);
    ST_AddLine(widget, line1);

    if (voxels_rendering)
    {
        static char line2[60];
        M_snprintf(line2, sizeof(line2), GRAY_S " Voxels %4d",
                   rendered_voxels);
        ST_AddLine(widget, line2);
    }
}

int speedometer;

static void UpdateSpeed(sbe_widget_t *widget, player_t *player)
{
    if (speedometer <= 0)
    {
        ST_ClearLines(widget);
        return;
    }

    ForceDoomFont(widget);

    static const double factor[] = {TICRATE, 2.4003, 525.0 / 352.0};
    static const char *units[] = {"ups", "km/h", "mph"};
    const int type = speedometer - 1;
    const mobj_t *mo = player->mo;
    const double dx = FIXED2DOUBLE(mo->x - mo->oldx);
    const double dy = FIXED2DOUBLE(mo->y - mo->oldy);
    const double dz = FIXED2DOUBLE(mo->z - mo->oldz);
    const double speed = sqrt(dx * dx + dy * dy + dz * dz) * factor[type];

    static char string[60];
    M_snprintf(string, sizeof(string), GRAY_S "%.*f " GREEN_S "%s",
               type && speed ? 1 : 0, speed, units[type]);
    SetLine(widget, string);
}

static void UpdateCmd(sbe_widget_t *widget)
{
    ST_ClearLines(widget);

    if (!STRICTMODE(hud_command_history))
    {
        return;
    }

    HU_BuildCommandHistory(widget);
}

// [Nugget]
static void UpdatePowers(sbe_widget_t *widget, player_t *player)
{
    ST_ClearLines(widget);

    static char string[64];

    memset(string, 0, sizeof(string));

    if (!WidgetEnabled(hud_power_timers))
    {
        return;
    }

    ForceDoomFont(widget);

    int offset = 0;
    
    // TO-DO: Multi-lined?

    #define POWERUP_TIMER(_power_, _powerdur_, _numicon_, _string_, _color_) \
        if (player->powers[_power_] > 0) \
        { \
          const boolean runout = POWER_RUNOUT(player->powers[_power_]); \
          \
          if (hud_allow_icons && ST_IconAvailable(_numicon_)) \
          { \
            offset += M_snprintf( \
              string + offset, sizeof(string) - offset, "\x1b%c%c\x1b%c ", \
              '0' + (runout ? CR_NONE : CR_BLACK), \
              (char) (HU_FONTEND + 1 + _numicon_), \
              '0' + (runout ? _color_ : CR_BLACK) \
            ); \
          } \
          else { \
            offset += M_snprintf( \
              string + offset, sizeof(string) - offset, "\x1b%c" _string_ " ", \
              '0' + (runout ? _color_ : CR_BLACK) \
            ); \
          } \
          \
          if (!hud_power_timers_notime) \
          { \
            offset += M_snprintf( \
              string + offset, sizeof(string) - offset, "%i\" ", \
              MIN(_powerdur_ / TICRATE, 1 + (player->powers[_power_] / TICRATE)) \
            ); \
          } \
        }

    POWERUP_TIMER(pw_invisibility,    INVISTICS,  3, "INVIS", CR_RED);
    POWERUP_TIMER(pw_invulnerability, INVULNTICS, 4, "INVUL", CR_GREEN);
    POWERUP_TIMER(pw_infrared,        INFRATICS,  5, "LIGHT", CR_BRICK);
    POWERUP_TIMER(pw_ironfeet,        IRONTICS,   6, "SUIT",  CR_GRAY);

    #undef POWERUP_TIMER

    if (offset <= 0) { return; }

    string[offset - 1] = '\0'; // Trim trailing space
    SetLine(widget, string);
}

// [crispy] print a bar indicating demo progress at the bottom of the screen
boolean ST_DemoProgressBar(boolean force)
{
    const int progress = video.unscaledw * playback_tic / playback_totaltics;
    static int old_progress = 0;

    if (old_progress < progress)
    {
        old_progress = progress;
    }
    else if (!force)
    {
        return false;
    }

    V_FillRect(0, SCREENHEIGHT - 2, progress, 1, v_darkest_color);
    V_FillRect(0, SCREENHEIGHT - 1, progress, 1, v_lightest_color);

    return true;
}

struct
{
    char **str;
    const int cr;
    const char *col;
} static const colorize_strings[] = {
    // [Woof!] colorize keycard and skull key messages
    {&s_GOTBLUECARD,     CR_BLUE2, "blue"  },
    {&s_GOTBLUESKUL,     CR_BLUE2, "blue"  },
    {&s_GOTREDCARD,      CR_RED,   "red"   },
    {&s_GOTREDSKULL,     CR_RED,   "red"   },
    {&s_GOTYELWCARD,     CR_GOLD,  "yellow"},
    {&s_GOTYELWSKUL,     CR_GOLD,  "yellow"},
    {&s_PD_BLUEC,        CR_BLUE2, "blue"  },
    {&s_PD_BLUEK,        CR_BLUE2, "blue"  },
    {&s_PD_BLUEO,        CR_BLUE2, "blue"  },
    {&s_PD_BLUES,        CR_BLUE2, "blue"  },
    {&s_PD_REDC,         CR_RED,   "red"   },
    {&s_PD_REDK,         CR_RED,   "red"   },
    {&s_PD_REDO,         CR_RED,   "red"   },
    {&s_PD_REDS,         CR_RED,   "red"   },
    {&s_PD_YELLOWC,      CR_GOLD,  "yellow"},
    {&s_PD_YELLOWK,      CR_GOLD,  "yellow"},
    {&s_PD_YELLOWO,      CR_GOLD,  "yellow"},
    {&s_PD_YELLOWS,      CR_GOLD,  "yellow"},

    // [Woof!] colorize multi-player messages
    {&s_HUSTR_PLRGREEN,  CR_GREEN, "Green:" },
    {&s_HUSTR_PLRINDIGO, CR_GRAY,  "Indigo:"},
    {&s_HUSTR_PLRBROWN,  CR_BROWN, "Brown:" },
    {&s_HUSTR_PLRRED,    CR_RED,   "Red:"   },
};

static char* PrepareColor(const char *str, const char *col)
{
    char *str_replace, col_replace[16];

    M_snprintf(col_replace, sizeof(col_replace),
               ORIG_S "%s" ORIG_S, col);
    str_replace = M_StringReplaceWord(str, col, col_replace);

    return str_replace;
}

static void UpdateColor(char *str, int cr)
{
    int i;
    int len = strlen(str);

    if (!message_colorized)
    {
        cr = CR_ORIG;
    }

    for (i = 0; i < len; ++i)
    {
        if (str[i] == '\x1b' && i + 1 < len)
        {
          str[i + 1] = '0'+cr;
          break;
        }
    }
}

void ST_InitWidgets(void)
{
    // [Woof!] prepare player messages for colorization
    for (int i = 0; i < arrlen(colorize_strings); i++)
    {
        *colorize_strings[i].str =
            PrepareColor(*colorize_strings[i].str, colorize_strings[i].col);
    }

    ST_ResetMessageColors();
}

void ST_ResetMessageColors(void)
{
    int i;

    for (i = 0; i < arrlen(colorize_strings); i++)
    {
        UpdateColor(*colorize_strings[i].str, colorize_strings[i].cr);
    }
}

sbarelem_t *st_time_elem = NULL, *st_cmd_elem = NULL;

boolean message_centered;
sbarelem_t *st_msg_elem = NULL;

static void ForceCenterMessage(sbarelem_t *elem)
{
    // [Nugget] NUGHUD
    if (ST_GetNughudOn())
    {
        if (nughud.message_defx)
        {
            if (message_centered)
            {
                elem->x_pos = SCREENWIDTH / 2;
                elem->alignment = sbe_h_middle;
            }
            else {
                elem->x_pos = 0;
                elem->alignment = sbe_h_left | sbe_wide_left;
            }
        }

        return;
    }

    static sbaralignment_t default_alignment;
    if (!st_msg_elem)
    {
        default_alignment = elem->alignment;
        st_msg_elem = elem;
    }

    elem->alignment = message_centered ? sbe_h_middle : default_alignment;
}

void ST_UpdateWidget(sbarelem_t *elem, player_t *player)
{
    sbe_widget_t *widget = elem->subtype.widget;

    switch (widget->type)
    {
        case sbw_message:
            ForceCenterMessage(elem);
            UpdateMessage(widget, player);
            break;
        case sbw_chat:
            UpdateChat(widget);
            break;
        case sbw_announce:
            UpdateAnnounceMessage(widget, player);
            break;
        case sbw_title:
            UpdateTitle(widget);
            break;

        case sbw_monsec:
            if (deathmatch)
                UpdateDM(widget);
            else
                UpdateMonSec(widget);
            break;
        case sbw_time:
            st_time_elem = elem;
            UpdateStTime(widget, player);
            break;
        case sbw_coord:
            UpdateCoord(widget, player);
            break;
        case sbw_fps:
            UpdateFPS(widget, player);
            break;
        case sbw_rate:
            UpdateRate(widget, player);
            break;
        case sbw_cmd:
            st_cmd_elem = elem;
            UpdateCmd(widget);
            break;
        case sbw_speed:
            UpdateSpeed(widget, player);
            break;

        // [Nugget]
        case sbw_powers: UpdatePowers(widget, player); break;

        default:
            break;
    }
}

void ST_BindHUDVariables(void)
{
  M_BindNum("hud_level_stats", &hud_level_stats, NULL,
            HUD_WIDGET_OFF, HUD_WIDGET_OFF, HUD_WIDGET_ALWAYS,
            ss_stat, wad_no,
            "Show level stats (kills, items, and secrets) widget (1 = On automap; "
            "2 = On HUD; 3 = Always)");
  M_BindNum("hud_stats_format", &hud_stats_format, NULL,
            STATSFORMAT_RATIO, STATSFORMAT_RATIO, NUM_STATSFORMATS-1, // [Nugget]
            ss_stat, wad_no,
            "Format of level stats (1 = Ratio; 2 = Boolean; 3 = Percent; 4 = Remaining; 5 = Count)");

  // [Nugget] /---------------------------------------------------------------

  M_BindNum("hud_stats_format_map", &hud_stats_format_map, NULL,
            STATSFORMAT_MATCHHUD, STATSFORMAT_MATCHHUD, NUM_STATSFORMATS-1, ss_stat, wad_no,
            "Format of level stats in automap (0 = Match HUD)");

  M_BindBool("hud_stats_kills", &hud_stats_show[SHOWSTATS_KILLS], NULL,
             true, ss_stat, wad_no,
             "Show kill count in the level-stats widget");

  M_BindBool("hud_stats_items", &hud_stats_show[SHOWSTATS_ITEMS], NULL,
             true, ss_stat, wad_no,
             "Show item count in the level-stats widget");

  M_BindBool("hud_stats_secrets", &hud_stats_show[SHOWSTATS_SECRETS], NULL,
             true, ss_stat, wad_no,
             "Show secrets count in the level-stats widget");

  M_BindBool("hud_stats_kills_map", &hud_stats_show_map[SHOWSTATS_KILLS], NULL,
             true, ss_stat, wad_no,
             "Show kill count in the automap's level-stats widget");

  M_BindBool("hud_stats_items_map", &hud_stats_show_map[SHOWSTATS_ITEMS], NULL,
             true, ss_stat, wad_no,
             "Show item count in the automap's level-stats widget");

  M_BindBool("hud_stats_secrets_map", &hud_stats_show_map[SHOWSTATS_SECRETS], NULL,
             true, ss_stat, wad_no,
             "Show secrets count in the automap's level-stats widget");

  // [Nugget] ---------------------------------------------------------------/

  M_BindNum("hud_level_time", &hud_level_time, NULL,
            HUD_WIDGET_OFF, HUD_WIDGET_OFF, HUD_WIDGET_ALWAYS,
            ss_stat, wad_no,
            "Show level time widget (1 = On automap; 2 = On HUD; 3 = Always)");
  M_BindNum("hud_player_coords", &hud_player_coords, NULL,
            HUD_WIDGET_AUTOMAP, HUD_WIDGET_OFF, HUD_WIDGET_ADVANCED,
            ss_stat, wad_no,
            "Show player coordinates widget (1 = On automap; 2 = On HUD; 3 = Always; 4 = Advanced)");
  M_BindBool("hud_command_history", &hud_command_history, NULL, false, ss_stat,
             wad_no, "Show command history widget");
  BIND_NUM(hud_command_history_size, 10, 1, HU_MAXMESSAGES,
           "Number of commands to display for command history widget");
  BIND_BOOL(hud_hide_empty_commands, true,
            "Hide empty commands from command history widget");

  // [Nugget] Extended
  M_BindBool("hud_time_use", &hud_time[TIMER_USE], NULL, false, ss_stat, wad_no,
             "Show split time when pressing the use-button");

  M_BindNum("hud_widget_font", &hud_widget_font, NULL,
            HUD_WIDGET_AUTOMAP, HUD_WIDGET_OFF, HUD_WIDGET_ALWAYS,
            ss_stat, wad_no,
            "Use standard Doom font for widgets (1 = On automap; 2 = On HUD; 3 "
            "= Always)");

  // [Nugget] /---------------------------------------------------------------

  M_BindBool("hud_time_teleport", &hud_time[TIMER_TELEPORT], NULL,
             false, ss_stat, wad_no,
             "Show split time when going through a teleporter");

  M_BindBool("hud_time_keypickup", &hud_time[TIMER_KEYPICKUP], NULL,
             false, ss_stat, wad_no,
             "Show split time when picking up a key");

  M_BindNum("hud_power_timers", &hud_power_timers, NULL,
            HUD_WIDGET_OFF, HUD_WIDGET_OFF, HUD_WIDGET_ALWAYS, ss_stat, wad_no,
            "Show powerup-timers widget (1 = On automap; 2 = On HUD; 3 = Always)");

  // (CFG-only)
  M_BindBool("hud_power_timers_notime", &hud_power_timers_notime, NULL,
             false, ss_none, wad_no,
             "Show only powerup names/icons in powerup-timers widget");

  // [Nugget] ---------------------------------------------------------------/

  // [Nugget]
  M_BindBool("hud_allow_icons", &hud_allow_icons, NULL,
             true, ss_stat, wad_yes,
             "Allow usage of icons for some labels in HUD widgets");

  M_BindNum("hudcolor_titl", &hudcolor_titl, NULL,
            CR_GOLD, CR_BRICK, CR_NONE, ss_none, wad_yes,
            "Color range used for automap level title");
  M_BindNum("hudcolor_xyco", &hudcolor_xyco, NULL,
            CR_GREEN, CR_BRICK, CR_NONE, ss_none, wad_yes,
            "Color range used for automap coordinates");

  // [Nugget] Extended HUD colors /-------------------------------------------

  M_BindNum("hudcolor_time_scale", &hudcolor_time_scale, NULL,
            CR_BLUE1, CR_BRICK, CR_NONE, ss_hudcol, wad_yes,
            "Color used for time scale (game-speed percent) in Time display");

  M_BindNum("hudcolor_total_time", &hudcolor_total_time, NULL,
            CR_GREEN, CR_BRICK, CR_NONE, ss_hudcol, wad_yes,
            "Color used for total level time in Time display");

  M_BindNum("hudcolor_time", &hudcolor_time, NULL,
            CR_GRAY, CR_BRICK, CR_NONE, ss_hudcol, wad_yes,
            "Color used for level time in Time display");

  M_BindNum("hudcolor_event_timer", &hudcolor_event_timer, NULL,
            CR_GOLD, CR_BRICK, CR_NONE, ss_hudcol, wad_yes,
            "Color used for event timer in Time display");

  M_BindNum("hudcolor_kills", &hudcolor_kills, NULL,
            CR_RED, CR_BRICK, CR_NONE, ss_hudcol, wad_yes,
            "Color used for Kills label in Stats display");

  M_BindNum("hudcolor_items", &hudcolor_items, NULL,
            CR_RED, CR_BRICK, CR_NONE, ss_hudcol, wad_yes,
            "Color used for Items label in Stats display");

  M_BindNum("hudcolor_secrets", &hudcolor_secrets, NULL,
            CR_RED, CR_BRICK, CR_NONE, ss_hudcol, wad_yes,
            "Color used for Secrets label in Stats display");

  M_BindNum("hudcolor_ms_incomp", &hudcolor_ms_incomp, NULL,
            CR_GRAY, CR_BRICK, CR_NONE, ss_hudcol, wad_yes,
            "Color used for incomplete milestones in Stats display");

  M_BindNum("hudcolor_ms_comp", &hudcolor_ms_comp, NULL,
            CR_BLUE1, CR_BRICK, CR_NONE, ss_hudcol, wad_yes,
            "Color used for complete milestones in Stats display");

  // [Nugget] ---------------------------------------------------------------/

  BIND_BOOL(show_messages, true, "Show messages");

  // [Nugget] Sound-only option
  M_BindNum("hud_secret_message", &hud_secret_message, NULL,
            SECRETMESSAGE_ON, SECRETMESSAGE_OFF, NUM_SECRETMESSAGE-1,
            ss_stat, wad_no,
            "Announce revealed secrets (0 = Off; 1 = On; 2 = Count; 3 = Sound only)");

  M_BindBool("hud_map_announce", &hud_map_announce, NULL,
            false, ss_stat, wad_no, "Announce map titles");

  // [Nugget] Announce milestone completion
  M_BindBool("announce_milestones", &announce_milestones, NULL,
            false, ss_stat, wad_yes,
            "Announce completion of milestones");

  M_BindBool("show_toggle_messages", &show_toggle_messages, NULL,
            true, ss_stat, wad_no, "Show toggle messages");
  M_BindBool("show_pickup_messages", &show_pickup_messages, NULL,
             true, ss_stat, wad_no, "Show pickup messages");
  M_BindBool("show_obituary_messages", &show_obituary_messages, NULL,
             true, ss_stat, wad_no, "Show obituaries");

  // [Nugget] (CFG-only)
  M_BindBool("show_save_messages", &show_save_messages, NULL,
             true, ss_none, wad_no,
             "Show save messages");

  // [Nugget] Restored menu item
  M_BindNum("hudcolor_obituary", &hudcolor_obituary, NULL,
            CR_GRAY, CR_BRICK, CR_NONE, ss_stat, wad_no,
            "Color range used for obituaries");

  M_BindBool("message_centered", &message_centered, NULL,
             false, ss_stat, wad_no, "Center messages horizontally");
  M_BindBool("message_colorized", &message_colorized, NULL,
             false, ss_stat, wad_no, "Colorize player messages");

  // [Nugget] /---------------------------------------------------------------

  // Message fadeout
  M_BindBool("message_fadeout", &message_fadeout, NULL,
             false, ss_stat, wad_yes,
             "Messages fade out");

  // Message flash
  M_BindBool("message_flash", &message_flash, NULL,
             false, ss_stat, wad_yes,
             "Messages flash when they first appear");

  M_BindNum("hud_msg_duration", &hud_msg_duration, NULL,
            0, 0, UL, ss_stat, wad_no,
            "Force duration of messages, in tics (0 = Don't force)");

  M_BindNum("hud_chat_duration", &hud_chat_duration, NULL,
            0, 0, UL, ss_stat, wad_no,
            "Force duration of chat messages, in tics (0 = Don't force)");

  M_BindNum("hud_msg_lines", &hud_msg_lines, NULL,
            1, 1, 8, ss_stat, wad_no,
            "Number of lines in message list shown normally");

  M_BindNum("hud_msg_total_lines", &hud_msg_total_lines, NULL,
            1, 1, 8, ss_stat, wad_no,
            "Number of lines in message list shown during message review");

  M_BindBool("hud_msg_group", &hud_msg_group, NULL,
             false, ss_stat, wad_no,
             "Group repeated messages together");

  // (CFG-only)
  BIND_BOOL(hud_msg_scrollup, true, "Message list scrolls upwards");

  // [Nugget] ---------------------------------------------------------------/

#define BIND_CHAT(num)                                                     \
    M_BindStr("chatmacro" #num, &chat_macros[(num)], HUSTR_CHATMACRO##num, \
              wad_yes, "Chat string associated with " #num " key")

  BIND_CHAT(0);
  BIND_CHAT(1);
  BIND_CHAT(2);
  BIND_CHAT(3);
  BIND_CHAT(4);
  BIND_CHAT(5);
  BIND_CHAT(6);
  BIND_CHAT(7);
  BIND_CHAT(8);
  BIND_CHAT(9);
}
