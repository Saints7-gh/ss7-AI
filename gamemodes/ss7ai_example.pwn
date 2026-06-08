// =============================================================================
//  lia_test.pwn — Test LIA Mode (include ss7-AI v1.0.0)
//  Commands: /lia, /lia dialog, /aistatus
// =============================================================================

#include <open.mp>
#include <ss7cmd> //recommended using ss7cmd
#include <ss7-AI>

public OnGameModeInit()
{
    print("═══════════════════════════════════════════════════════════");
    print("  SS7-AI v1.0.0 — LIA Mode Test");
    print("  Commands:");
    print("    /lia          - Toggle LIA chat mode");
    print("    /lia dialog   - Open LIA dialog mode");
    print("    /aistatus     - Check AI status");
    print("═══════════════════════════════════════════════════════════");
    ss7AI_CacheServerInfo();
    return 1;
}

public OnPlayerConnect(playerid)
{
    new name[MAX_PLAYER_NAME + 1];
    GetPlayerName(playerid, name, sizeof(name));
    printf("[LIA] Player %s (ID:%d) connected", name, playerid);
    return 1;
}

public OnPlayerDisconnect(playerid, reason)
{
    new name[MAX_PLAYER_NAME + 1];
    GetPlayerName(playerid, name, sizeof(name));
    printf("[LIA] Player %s (ID:%d) disconnected", name, playerid);
    
    ss7AI_OnPlayerDisconnect(playerid);  // include punya cleanup
    return 1;
}

CMD:lia(playerid, params[])
{
    p_s;
    new opt[16];
    
    if (!s_ex(params, opt))
    {
        // Toggle LIA chat mode
        Lia_Toggle(playerid);
        return 1;
    }
    
    if (strcmp(opt, "dialog", true) == 0)
    {
        Lia_OpenDialog(playerid);
        return 1;
    }
    
    return ss7_SendSyntax(playerid, "/lia [dialog]");
}

CMD:aistatus(playerid, params[])
{
    #pragma unused params
    if (ss7AI_IsWaiting(playerid))
        ss7_SendInfo(playerid, "[AI] Status: Waiting for response...");
    else if (Lia_IsActive(playerid))
        ss7_SendInfo(playerid, "[LIA] Status: Active (chat mode)");
    else
        ss7_SendSuccess(playerid, "[AI] Status: Ready");
    return 1;
}

public OnPlayerText(playerid, text[])
{
    if (Lia_IsActive(playerid))
        return Lia_HandleText(playerid, text);
    
    return 1;
}

public OnDialogResponse(playerid, dialogid, response, listitem, inputtext[])
{
    if (Lia_OnDialogResponse(playerid, dialogid, response, inputtext))
        return 1;
    
    //your code etc...
    return 0;
}

main()
{
    print("[LIA] LIA Mode Test Ready — /lia to start");
}