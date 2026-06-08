/*   
                                                                  
 ad88888ba    ad88888ba  888888888888            db         88  
d8"     "8b  d8"     "8b         ,8P'           d88b        88  
Y8,          Y8,                d8"            d8'`8b       88  
`Y8aaaaa,    `Y8aaaaa,        ,8P'            d8'  `8b      88  
  `"""""8b,    `"""""8b,     d8"  aaaaaaaa   d8YaaaaY8b     88  
        `8b          `8b   ,8P'   """"""""  d8""""""""8b    88  
Y8a     a8P  Y8a     a8P  d8"              d8'        `8b   88  
 "Y88888P"    "Y88888P"  8P'              d8'          `8b  88  


 * SS7-AI v1.0.0
 * Zero-plugin AI Integration for open.mp / SA-MP
 *
 * Features:
 *   - Lia AI Mode — private chat session per player
 *   - Lia Dialog Mode
 *
 * Dependencies:
 *   - Python 3.8+ with ss7ai_bridge.py running
 *   - ss7cmd (recommended for command handling)
 *
 * Commands (handle in your gamemode):
 *   /lia          — Toggle Lia chat mode ON/OFF
 *   /lia dialog   — Open Lia dialog input loop
 *
 * Usage (public API, backward-compatible):
 *   #include <ss7-AI>
 *   ss7AI_Ask(playerid, "question", "Callback");
 *   ss7AI_AskDialog(playerid, "question", "Title", "Callback");
 *   Lia_Toggle(playerid);
 *   Lia_OpenDialog(playerid);
 *
 * Author:  Saints7
 * License: MIT
 * Repo:    https://github.com/Saints7-gh/ss7ai
 */                                


#if defined _ss7ai_included
    #endinput
#endif
#define _ss7ai_included

#if !defined SS7AI_POLL_MS
    #define SS7AI_POLL_MS           500
#endif
#if !defined SS7AI_TIMEOUT_MS
    #define SS7AI_TIMEOUT_MS        15000
#endif
#if !defined SS7AI_DIALOG_ID
    #define SS7AI_DIALOG_ID         9100
#endif
#if !defined SS7AI_LIA_DIALOG_ID
    #define SS7AI_LIA_DIALOG_ID     9101
#endif
#if !defined SS7AI_MAX_RESPONSE
    #define SS7AI_MAX_RESPONSE      1024
#endif
#if !defined SS7AI_MAX_PROMPT
    #define SS7AI_MAX_PROMPT        200
#endif

#if !defined SS7AI_COLOR_RESPONSE
    #define SS7AI_COLOR_RESPONSE    0x66CCFFFF
#endif
#if !defined SS7AI_COLOR_ERROR
    #define SS7AI_COLOR_ERROR       0xFF4444FF
#endif
#if !defined SS7AI_COLOR_WAIT
    #define SS7AI_COLOR_WAIT        0xFFCC00FF
#endif
#if !defined SS7AI_COLOR_LIA
    #define SS7AI_COLOR_LIA         0x00E5FFFF
#endif
#if !defined SS7AI_COLOR_LIA_INFO
    #define SS7AI_COLOR_LIA_INFO    0xAAFFEEFF
#endif

#define SS7AI_MODE_CHAT             0
#define SS7AI_MODE_DIALOG           1
#define SS7AI_MODE_BOTH             2
#define SS7AI_MODE_LIA_CHAT         3
#define SS7AI_MODE_LIA_DIALOG       4

static g_ss7ai_serverName[64];
static g_ss7ai_gamemodeName[64];
static g_ss7ai_serverIp[24];
static g_ss7ai_reqId            = 0;
static g_ss7ai_playerReqId      [MAX_PLAYERS];
static g_ss7ai_startTick        [MAX_PLAYERS];
static g_ss7ai_callback         [MAX_PLAYERS][64];
static g_ss7ai_mode             [MAX_PLAYERS];
static g_ss7ai_dialogTitle      [MAX_PLAYERS][64];
static bool:g_ss7ai_waiting     [MAX_PLAYERS];

static bool:g_lia_active        [MAX_PLAYERS];
static bool:g_lia_dialogLoop    [MAX_PLAYERS];

stock ss7AI_Ask(playerid, const prompt[], const callback[])
{
    return _ss7AI_Send(playerid, prompt, callback, SS7AI_MODE_CHAT, "SS7-AI Response");
}

stock ss7AI_AskDialog(playerid, const prompt[], const dialogTitle[], const callback[])
{
    return _ss7AI_Send(playerid, prompt, callback, SS7AI_MODE_DIALOG, dialogTitle);
}

stock ss7AI_AskBoth(playerid, const prompt[], const dialogTitle[], const callback[])
{
    return _ss7AI_Send(playerid, prompt, callback, SS7AI_MODE_BOTH, dialogTitle);
}

stock bool:ss7AI_IsWaiting(playerid)
{
    return g_ss7ai_waiting[playerid];
}

stock ss7AI_CacheServerInfo()
{
    GetConsoleVarAsString("name", g_ss7ai_serverName, sizeof(g_ss7ai_serverName));
    strcat(g_ss7ai_gamemodeName, "LIA", sizeof(g_ss7ai_gamemodeName));
    strcat(g_ss7ai_serverIp,     "127.0.0.1", sizeof(g_ss7ai_serverIp));
    if (g_ss7ai_serverIp[0] == '\0')
        strcat(g_ss7ai_serverIp, "127.0.0.1", sizeof(g_ss7ai_serverIp));
}

stock bool:Lia_Toggle(playerid)
{
    if (g_lia_active[playerid])
    {
        g_lia_active[playerid]     = false;
        g_lia_dialogLoop[playerid] = false;
        SendClientMessage(playerid, SS7AI_COLOR_LIA_INFO,
            "{AAFFEE}[LIA] {FFFFFF}Lia Mode disabled. Back to normal chat.");
        return false;
    }
    else
    {
        g_lia_active[playerid] = true;
        SendClientMessage(playerid, SS7AI_COLOR_LIA, "{00E5FF}--------------------------------------------------------------------");
        SendClientMessage(playerid, SS7AI_COLOR_LIA, "{00E5FF}[LIA] {FFFFFF}Lia Mode activated! Type your message, Lia will reply.");
        SendClientMessage(playerid, SS7AI_COLOR_LIA, "{00E5FF}[LIA] {AAAAAA}Type /lia again to disable this mode.");
        SendClientMessage(playerid, SS7AI_COLOR_LIA, "{00E5FF}--------------------------------------------------------------------");
        SendClientMessage(playerid, SS7AI_COLOR_LIA, "");
        return true;
    }
}

stock bool:Lia_IsActive(playerid)
{
    return g_lia_active[playerid];
}

stock Lia_HandleText(playerid, const text[])
{
    if (g_ss7ai_waiting[playerid])
    {
        SendClientMessage(playerid, SS7AI_COLOR_WAIT,
            "{FFCC00}[LIA] {FFFFFF}Please wait for the previous response to finish.");
        return 0;
    }

    static playerName[MAX_PLAYER_NAME + 1];
    static selfMsg[160];
    GetPlayerName(playerid, playerName, sizeof(playerName));
    
    format(selfMsg, sizeof(selfMsg), "{FFFFFF}%s{AAAAAA}: {FFFFFF}%s", playerName, text);
    SendClientMessage(playerid, 0xFFFFFFFF, selfMsg);

    SendClientMessage(playerid, SS7AI_COLOR_LIA,
        "{00E5FF}> {AAFFEE}Waiting for Lia to respond...");

    _ss7AI_Send(playerid, text, "_Lia_OnChatResponse", SS7AI_MODE_LIA_CHAT, "Lia");
    return 0;
}

stock Lia_OpenDialog(playerid)
{
    if (!g_lia_active[playerid]) g_lia_active[playerid] = true;
    g_lia_dialogLoop[playerid] = true;
    _Lia_ShowInputDialog(playerid);
}

stock _Lia_ShowInputDialog(playerid, const liaResponse[] = "")
{
    new body[1024];
    
    if (liaResponse[0] != '\0')
    {
        new wrapped[1024];
        wrapped[0] = '\0';
        new rlen   = strlen(liaResponse);
        new rstart = 0;
        new lineW  = 72;
        new linePos = 0;

        while (rstart < rlen)
        {
            new c = liaResponse[rstart];

            if (c == '\n')
            {
                strcat(wrapped, "\n", sizeof(wrapped));
                rstart++;
                linePos = 0;
                continue;
            }

            new rend = rstart + (lineW - linePos);
            if (rend >= rlen) rend = rlen;

            new lastSp = rend;
            if (rend < rlen)
            {
                while (lastSp > rstart && liaResponse[lastSp] != ' ' && liaResponse[lastSp] != '\n') lastSp--;
                if (lastSp <= rstart) lastSp = rend;
            }

            new tmp[128];
            strmid(tmp, liaResponse, rstart, lastSp, sizeof(tmp));
            strcat(wrapped, tmp, sizeof(wrapped));

            rstart = lastSp;
            while (liaResponse[rstart] == ' ') rstart++;

            if (rstart < rlen && liaResponse[rstart] != '\n')
            {
                strcat(wrapped, "\n", sizeof(wrapped));
                linePos = 0;
            }
        }

        format(body, sizeof(body),
            "{00E5FF}[LIA]{FFFFFF}: %s\n\n{AAAAAA}Press {FF4444}Cancel {AAAAAA}to end the conversation.",
            wrapped);
    }
    else
    {
        format(body, sizeof(body),
            "{AAFFEE}Hi! I'm Lia.\n{FFFFFF}Write your message below and I'll reply.\n\n{AAAAAA}Press {FF4444}Cancel {AAAAAA}to end the conversation.");
    }
    
    ShowPlayerDialog(playerid, SS7AI_LIA_DIALOG_ID, DIALOG_STYLE_INPUT,
        "{00E5FF}♦ Lia AI {FFFFFF}— Private Chat",
        body,
        "Send", "Cancel");
}

stock Lia_OnDialogResponse(playerid, dialogid, response, const inputtext[])
{
    if (dialogid == SS7AI_LIA_DIALOG_ID)
    {
        if (!response)
        {
            g_lia_dialogLoop[playerid] = false;
            SendClientMessage(playerid, SS7AI_COLOR_LIA_INFO,
                "{AAFFEE}[LIA] {FFFFFF}Chat dialog ended.");
            return 1;
        }

        if (inputtext[0] == '\0')
        {
            _Lia_ShowInputDialog(playerid);
            return 1;
        }

        if (g_ss7ai_waiting[playerid])
        {
            SendClientMessage(playerid, SS7AI_COLOR_WAIT,
                "{FFCC00}[LIA] {FFFFFF}Waiting for Lia to respond...");
            _Lia_ShowInputDialog(playerid);
            return 1;
        }

        static playerName[MAX_PLAYER_NAME + 1];
        static selfMsg[256];
        GetPlayerName(playerid, playerName, sizeof(playerName));
        
        format(selfMsg, sizeof(selfMsg), "{FFFFFF}%s{AAAAAA}: {FFFFFF}%s", playerName, inputtext);
        SendClientMessage(playerid, 0xFFFFFFFF, selfMsg);

        SendClientMessage(playerid, SS7AI_COLOR_LIA,
            "{00E5FF}> {AAFFEE}Waiting for Lia to respond...");

        _ss7AI_Send(playerid, inputtext, "_Lia_OnChatResponse", SS7AI_MODE_LIA_DIALOG, "Lia");
        return 1;
    }
    return 0;
}

static stock _ss7AI_Send(playerid, const prompt[], const callback[], mode, const dialogTitle[])
{
    if (g_ss7ai_waiting[playerid])
    {
        SendClientMessage(playerid, SS7AI_COLOR_WAIT,
            "{FFCC00}[AI] {FFFFFF}Please wait for the previous response to finish.");
        return 0;
    }

    new plen = strlen(prompt);
    if (plen == 0 || plen > SS7AI_MAX_PROMPT)
    {
        SendClientMessage(playerid, SS7AI_COLOR_ERROR,
            "{FF4444}[AI] {FFFFFF}Invalid prompt length.");
        return 0;
    }

    g_ss7ai_reqId++;

    static safePrompt[SS7AI_MAX_PROMPT + 1];
    new pi = 0, po = 0, c;
    new limit = SS7AI_MAX_PROMPT - 1;
    
    while (pi < plen && po < limit)
    {
        c = prompt[pi++];
        if      (c == '"')  safePrompt[po++] = '\'';
        else if (c == '\\') safePrompt[po++] = '/';
        else                safePrompt[po++] = c;
    }
    safePrompt[po] = '\0';

    static playerName[MAX_PLAYER_NAME + 1];
    GetPlayerName(playerid, playerName, sizeof(playerName));

    static reqFile[64];
    format(reqFile, sizeof(reqFile), "ss7ai_req_%d.json", g_ss7ai_reqId);

    new File:f = fopen(reqFile, io_write);
    if (!f)
    {
        SendClientMessage(playerid, SS7AI_COLOR_ERROR,
            "{FF4444}[AI] {FFFFFF}Bridge error: failed to write request.");
        return 0;
    }

    new serverName[64], gamemodeName[64], serverIp[24];
    strcat(serverName,   g_ss7ai_serverName,   sizeof(serverName));
    strcat(gamemodeName, g_ss7ai_gamemodeName, sizeof(gamemodeName));
    strcat(serverIp,     g_ss7ai_serverIp,     sizeof(serverIp));
    
    new onlinePlayers = 0;
    for (new i = 0; i < MAX_PLAYERS; i++) if (IsPlayerConnected(i)) onlinePlayers++;

    static json[768], jsonPart1[400], jsonPart2[400];
    format(jsonPart1, sizeof(jsonPart1),
        "{\"id\":%d,\"playerid\":%d,\"player_name\":\"%s\",\"prompt\":\"%s\",\"max_chars\":%d,",
        g_ss7ai_reqId, playerid, playerName, safePrompt, SS7AI_MAX_RESPONSE);
        
    format(jsonPart2, sizeof(jsonPart2),
        "\"server_name\":\"%s\",\"gamemode\":\"%s\",\"server_ip\":\"%s\",\"online\":%d,\"maxplayers\":%d}",
        serverName, gamemodeName, serverIp, onlinePlayers, GetMaxPlayers());
        
    json[0] = '\0';
    strcat(json, jsonPart1, sizeof(json));
    strcat(json, jsonPart2, sizeof(json));
    fwrite(f, json);
    fclose(f);

    g_ss7ai_playerReqId [playerid] = g_ss7ai_reqId;
    g_ss7ai_startTick   [playerid] = GetTickCount();
    g_ss7ai_mode        [playerid] = mode;
    g_ss7ai_waiting     [playerid] = true;
    strmid(g_ss7ai_callback    [playerid], callback,    0, 63);
    strmid(g_ss7ai_dialogTitle [playerid], dialogTitle, 0, 63);

    SetTimerEx("_ss7AI_Poll", SS7AI_POLL_MS, false, "i", playerid);
    return g_ss7ai_reqId;
}

forward _ss7AI_Poll(playerid);
public  _ss7AI_Poll(playerid)
{
    if (!g_ss7ai_waiting[playerid]) return;

    new reqId = g_ss7ai_playerReqId[playerid];

    if (GetTickCount() - g_ss7ai_startTick[playerid] > SS7AI_TIMEOUT_MS)
    {
        g_ss7ai_waiting[playerid] = false;
        SendClientMessage(playerid, SS7AI_COLOR_ERROR,
            "{FF4444}[LIA] {FFFFFF}Request timed out. Please try again.");
        CallLocalFunction(g_ss7ai_callback[playerid], "isib", playerid, "Request timed out.", reqId, false);

        static reqFile[64];
        format(reqFile, sizeof(reqFile), "ss7ai_req_%d.json", reqId);
        if (fexist(reqFile)) fremove(reqFile);
        return;
    }

    static resFile[64];
    format(resFile, sizeof(resFile), "ss7ai_res_%d.json", reqId);

    if (!fexist(resFile))
    {
        SetTimerEx("_ss7AI_Poll", SS7AI_POLL_MS, false, "i", playerid);
        return;
    }

    new File:f = fopen(resFile, io_read);
    if (!f)
    {
        SetTimerEx("_ss7AI_Poll", SS7AI_POLL_MS, false, "i", playerid);
        return;
    }

    static json[2048];
    static response[SS7AI_MAX_RESPONSE + 128];
    json[0] = '\0';
    response[0] = '\0';
    
    new readLen = 0, ch;
    new jsonLimit = sizeof(json) - 1;
    while ((ch = fgetchar(f, false)) != -1 && readLen < jsonLimit)
        json[readLen++] = ch;
    json[readLen] = '\0';
    fclose(f);
    fremove(resFile);

    g_ss7ai_waiting[playerid] = false;

    static status[8];
    _ss7AI_ParseField(json, "status", status, sizeof(status));
    new bool:success = (strcmp(status, "ok") == 0);

    static resIdStr[12];
    _ss7AI_ParseField(json, "id", resIdStr, sizeof(resIdStr));
    if (strval(resIdStr) != reqId)
    {
        fremove(resFile);
        return;
    }

    _ss7AI_ParseField(json, "response", response, sizeof(response));

    if (!success || response[0] == '\0')
    {
        SendClientMessage(playerid, SS7AI_COLOR_ERROR,
            "{FF4444}[LIA] {FFFFFF}Error: no response received.");
        CallLocalFunction(g_ss7ai_callback[playerid], "isib", playerid, "No response.", reqId, false);
        return;
    }

    new mode = g_ss7ai_mode[playerid];

    if (mode == SS7AI_MODE_LIA_CHAT || mode == SS7AI_MODE_LIA_DIALOG)
    {
        static liaMsg[1040];
        format(liaMsg, sizeof(liaMsg), "{00E5FF}[LIA] {FFFFFF}%s", response);
        SLM(playerid, 0xFFFFFFFF, liaMsg);
    }
    else if (mode == SS7AI_MODE_CHAT || mode == SS7AI_MODE_BOTH)
    {
        static chatMsg[1032];
        format(chatMsg, sizeof(chatMsg), "{66CCFF}[AI] {FFFFFF}%s", response);
        SLM(playerid, 0xFFFFFFFF, chatMsg);
    }

    if (mode == SS7AI_MODE_DIALOG || mode == SS7AI_MODE_BOTH)
    {
        SLD(playerid, g_ss7ai_dialogTitle[playerid], response, "OK");
    }

    if (mode == SS7AI_MODE_LIA_DIALOG && g_lia_dialogLoop[playerid])
    {
        _Lia_ShowInputDialog(playerid, response);
    }

    CallLocalFunction(g_ss7ai_callback[playerid], "isib", playerid, response, reqId, true);
}

stock _ss7AI_ParseField(const json[], const field[], dest[], maxlen)
{
    new key[72];
    format(key, sizeof(key), "\"%s\":", field);

    new pos = strfind(json, key);
    if (pos == -1) return;

    pos += strlen(field) + 3; 
    while (json[pos] == ' ' || json[pos] == '\t') pos++;

    if (json[pos] == '"')
    {
        pos++;
        new i = 0, c;
        new limit = maxlen - 1;

        while (i < limit)
        {
            c = json[pos++];
            if (c == '"' || c == '\0') break;

            if (c == '\\')
            {
                c = json[pos++];
                if      (c == 'n') dest[i++] = '\n';
                else if (c == 't') dest[i++] = '\t';
                else               dest[i++] = c;
            }
            else
            {
                dest[i++] = c;
            }
        }
        dest[i] = '\0';
    }
    else
    {
        new i = 0, c;
        new limit = maxlen - 1;
        while (i < limit)
        {
            c = json[pos++];
            if (c == ',' || c == '}' || c == '\0') break;
            dest[i++] = c;
        }
        dest[i] = '\0';
    }
}

stock SLM(playerid, color, const message[])
{
    static msg[145]; 
    new len   = strlen(message);
    new start = 0;
    new bool:firstLine = true;

    while (start < len)
    {
        new end = start + (firstLine ? 85 : 90);

        if (end >= len)
        {
            strmid(msg, message, start, len, sizeof(msg));
            SendClientMessage(playerid, color, msg);
            break;
        }

        new lastSpace = end;
        while (lastSpace > start && message[lastSpace] != ' ') lastSpace--;
        if (lastSpace <= start) lastSpace = end;

        strmid(msg, message, start, lastSpace, sizeof(msg));
        SendClientMessage(playerid, color, msg);

        firstLine = false;
        if (lastSpace == end) start = end;
        else 
        { 
            start = lastSpace; 
            while (message[start] == ' ') start++; 
        }
    }
}

stock SLD(playerid, const title[], const message[], const button[])
{
    static fullText[4096];
    fullText[0] = '\0';

    new len   = strlen(message);
    new start = 0;
    new lineW = 60;

    while (start < len)
    {
        new end = start + lineW;
        if (end >= len) end = len;

        new lastSpace = end;
        if (end < len)
        {
            while (lastSpace > start && message[lastSpace] != ' ') lastSpace--;
            if (lastSpace <= start) lastSpace = end;
        }

        new temp[128];
        strmid(temp, message, start, lastSpace, sizeof(temp));
        strcat(fullText, temp,  sizeof(fullText));
        strcat(fullText, "\n", sizeof(fullText));

        start = (lastSpace == end) ? end : lastSpace;
        while (message[start] == ' ') start++;
    }

    ShowPlayerDialog(playerid, SS7AI_DIALOG_ID, DIALOG_STYLE_MSGBOX,
        title, fullText, button, "");
}

stock ss7AI_OnPlayerDisconnect(playerid)
{
    static clearFile[48];
    format(clearFile, sizeof(clearFile), "ss7ai_clear_%d.json", playerid);
    new File:f = fopen(clearFile, io_write);
    if (f) { fwrite(f, "clear"); fclose(f); }

    g_ss7ai_waiting     [playerid] = false;
    g_ss7ai_playerReqId [playerid] = 0;
    g_ss7ai_startTick   [playerid] = 0;
    g_ss7ai_callback    [playerid][0] = '\0';
    g_lia_active        [playerid] = false;
    g_lia_dialogLoop    [playerid] = false;
}