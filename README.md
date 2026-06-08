# 🤖 SS7-AI — Zero-Plugin AI Integration for open.mp / SA-MP

<div align="center">

```
 ad88888ba    ad88888ba  888888888888            db         88  
d8"     "8b  d8"     "8b         ,8P'           d88b        88  
Y8,          Y8,                d8"            d8'`8b       88  
`Y8aaaaa,    `Y8aaaaa,        ,8P'            d8'  `8b      88  
  `"""""8b,    `"""""8b,     d8"  aaaaaaaa   d8YaaaaY8b     88  
        `8b          `8b   ,8P'   """"""""  d8""""""""8b    88  
Y8a     a8P  Y8a     a8P  d8"              d8'        `8b   88  
 "Y88888P"    "Y88888P"  8P'              d8'          `8b  88  
```

**LIA — Your server's virtual companion**

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![Platform](https://img.shields.io/badge/Platform-open.mp%20%7C%20SA--MP-blue)](https://open.mp)
[![Python](https://img.shields.io/badge/Python-3.8+-green.svg)](https://python.org)
[![Version](https://img.shields.io/badge/Version-1.0.0-red)](https://github.com/Saints7-gh/ss7ai)

**Zero plugins · Just include & run**

</div>

---

## ✨ Features

| Feature | Description |
|---------|-------------|
| **🔌 Zero Plugin** | Pure PAWN include + Python bridge — no `.so` or `.dll` files needed |
| **💬 LIA Chat Mode** | Private AI chat session per player (toggle with `/lia`) |
| **📱 LIA Dialog Mode** | Chat via dialog box with auto-continuation (`/lia dialog`) |
| **🌍 Multi-Language** | Auto-detects player's language — replies in same language |
| **🎭 Real Personality** | Warm, witty, sometimes sarcastic — not a yes-machine |
| **📝 Smart Word-Wrap** | Word-wrapping for both chat messages and dialogs |
| **⚡ Async Processing** | Non-blocking — players never wait for each other |
| **🧠 Per-Player Memory** | Remembers last 5 exchanges per player per session |

---

## 🎮 What is LIA?

**LIA** is not just a chatbot. She's a **virtual companion** built into your server.

She has:
- Real personality (warm, playful, sometimes sarcastic)
- Short, natural replies — 1 to 3 sentences
- Memory of your current conversation
- Awareness of your server (name, gamemode, player count, time)
- Zero "AI assistant" energy — she's a friend, not a tool

> *"You are NOT an AI, NOT a bot, NOT an assistant. You are a friend."*  
> — LIA's System Prompt

---

## 📋 Requirements

| Requirement | Version |
|-------------|---------|
| **open.mp** or **SA-MP** | Any version |
| **Python** | 3.8 or higher |
| **requests** library | `pip install requests` |
| **Any OpenAI-compatible API key** | OpenRouter, Groq, OpenAI, Ollama, etc. |

---

## 🚀 Installation

### 1. Install Python dependencies

```bash
pip install requests
```

### 2. Place the files

```
server/                          ← open.mp / SA-MP server root
├── ss7ai.py                     ← Python bridge  ← copy this
├── ss7ai.bin                    ← Encoded bridge  ← copy this
├── scriptfiles/
│   ├── ss7ai_config.json        ← Auto-created on first run
│   ├── ss7ai.log                ← Auto-created (bridge logs)
│   └── ss7ai.pid                ← Auto-created (process ID)
└── gamemodes/
    └── your_gamemode.pwn

qawno/include/
└── ss7-AI.inc                   ← PAWN include  ← copy this
```

> `ss7ai_config.json` is created automatically on first run. Just fill in your API key.

### 3. Add to your gamemode

```pawn
#include <ss7-AI>

public OnGameModeInit()
{
    ss7AI_CacheServerInfo();  // Cache server name, IP, gamemode, etc.
    return 1;
}

public OnPlayerDisconnect(playerid, reason)
{
    ss7AI_OnPlayerDisconnect(playerid);  // Clear player session data
    return 1;
}

public OnPlayerText(playerid, text[])
{
    if (Lia_IsActive(playerid))
        return Lia_HandleText(playerid, text);  // Route to LIA
    return 1;
}

public OnDialogResponse(playerid, dialogid, response, listitem, inputtext[])
{
    if (Lia_OnDialogResponse(playerid, dialogid, response, inputtext))
        return 1;
    // Your other dialogs here...
    return 0;
}

CMD:lia(playerid, params[])
{
    if (isnull(params))
    {
        Lia_Toggle(playerid);
        return 1;
    }
    if (!strcmp(params, "dialog", true))
    {
        Lia_OpenDialog(playerid);
        return 1;
    }
    return ss7_Syntax(playerid, "/lia or /lia dialog");
}
```

### 4. Configure AI provider

Edit `scriptfiles/ss7ai_config.json`:

```json
{
    "provider":    "OpenRouter",
    "api_key":     "your-api-key-here",
    "api_url":     "https://openrouter.ai/api/v1/chat/completions",
    "model":       "your-ai-model",
    "timeout_sec": 20
}
```

The bridge **hot-reloads** this config every ~60 seconds — no restart needed after editing.

**Example providers usage:**

| Provider | `api_url` | Example `model` |
|----------|-----------|-----------------|
| OpenRouter | `https://openrouter.ai/api/v1/chat/completions` | `deepseek/deepseek-r1-free` |
| OpenAI | `https://api.openai.com/v1/chat/completions` | `gpt-4o-mini` |
| Groq | `https://api.groq.com/openai/v1/chat/completions` | `llama3-8b-8192` |
| Together AI | `https://api.together.xyz/v1/chat/completions` | `meta-llama/Llama-3-8b-chat-hf` |
| Ollama (local) | `http://localhost:11434/v1/chat/completions` | `llama3` |

> **Free options:** OpenRouter and Groq both offer free tiers with capable models.

---

## ▶️ Running the Bridge

> ⚡ The bridge **must be running** before players connect!

### Linux

**Foreground (development):**
```bash
cd /path/to/your/server
python3 ss7ai.py
```

**Background with nohup (recommended for production):**
```bash
nohup python3 ss7ai.py > /dev/null 2>&1 &
```

**Background with screen:**
```bash
screen -S ss7ai
python3 ss7ai.py
# Press Ctrl+A then D to detach
# screen -r ss7ai to reattach
```

**Using bridge (loader + payload):**
```bash
python3 ss7ai.py
```
*(Both `ss7ai.py` and `ss7ai.bin` must be in the same folder)*

**With custom scriptfiles path:**
```bash
python3 ss7ai.py /path/to/scriptfiles
```

---

### Windows

**Foreground (development):**
```cmd
cd C:\path\to\your\server
python ss7ai.py
```

**Run minimized in background:**
```cmd
start /min python ss7ai.py
```

---

## 🎮 In-Game Commands

| Command | Action |
|---------|--------|
| `/lia` | Toggle LIA chat mode ON / OFF |
| `/lia dialog` | Open LIA private dialog window |

**Chat Mode flow:**
```
> /lia
  [LIA] Lia Mode activated. Type your message, I'll reply.
  [LIA] Type /lia again to turn me off.

> hey lia, how many players are online?
  [You]: hey lia, how many players are online?
  > Waiting for Lia...
  [LIA] Just you right now. Pretty quiet tonight.

> /lia
  [LIA] Lia Mode disabled. Back to normal chat.
```

**Dialog Mode flow:**
```
> /lia dialog
  ┌─ ♦ Lia AI — Private Chat ──────────────────────┐
  │ [LIA]: Hey! What's up?                          │
  │                                                  │
  │ Press Cancel to end the conversation.            │
  │ ┌──────────────────────────────────────────────┐ │
  │ │ _                                            │ │
  │ └──────────────────────────────────────────────┘ │
  │            [ Send ]        [ Cancel ]            │
  └──────────────────────────────────────────────────┘
```
Dialog stays open after each reply. Press **Cancel** to end the session.

---

## 🔧 Public API Reference

### Core

```pawn
ss7AI_CacheServerInfo()               // Call in OnGameModeInit
ss7AI_OnPlayerDisconnect(playerid)    // Call in OnPlayerDisconnect
ss7AI_IsWaiting(playerid) → bool      // Is player waiting for a response?
```

### Direct AI calls

```pawn
// Chat message response only
ss7AI_Ask(playerid, "Hello!", "OnAIResponse");

// Dialog response only
ss7AI_AskDialog(playerid, "Tell me about yourself", "LIA", "OnAIResponse");

// Both chat + dialog
ss7AI_AskBoth(playerid, "What's new?", "Server Info", "OnAIResponse");
```

### LIA functions

```pawn
Lia_Toggle(playerid) → bool                               // Toggle chat mode, returns new state
Lia_IsActive(playerid) → bool                             // Is LIA chat mode on?
Lia_HandleText(playerid, text[])                          // Feed player chat to LIA
Lia_OpenDialog(playerid)                                  // Open LIA dialog loop
Lia_OnDialogResponse(playerid, dialogid, response, inputtext[])  // Wire to OnDialogResponse
```

### Callback template

```pawn
forward OnAIResponse(playerid, response[], reqId, bool:success);
public  OnAIResponse(playerid, response[], reqId, bool:success)
{
    if (success)
        SendClientMessage(playerid, 0x66CCFFFF, response);
    else
        SendClientMessage(playerid, 0xFF4444FF, "[LIA] No response. Try again.");
    return 1;
}
```

---

## ⚙️ Configurable Defines

Override these **before** `#include <ss7-AI>`:

```pawn
#define SS7AI_POLL_MS        500    // How often PAWN polls for a response (ms)
#define SS7AI_TIMEOUT_MS     15000  // Max wait time before timeout (ms)
#define SS7AI_MAX_RESPONSE   1024   // Max response buffer length (chars)
#define SS7AI_MAX_PROMPT     200    // Max prompt input length (chars)

// Dialog IDs (change if they conflict with your gamemode)
#define SS7AI_DIALOG_ID      9100
#define SS7AI_LIA_DIALOG_ID  9101

// Colors
#define SS7AI_COLOR_LIA      0x00E5FFFF
#define SS7AI_COLOR_ERROR    0xFF4444FF
#define SS7AI_COLOR_WAIT     0xFFCC00FF
```

---

## 🛠️ Troubleshooting

| Issue | Solution |
|-------|----------|
| Bridge not responding | Check if Python is running: `ps aux \| grep ss7ai` |
| `"failed to write request"` | Verify `scriptfiles/` exists and is writable |
| API 401 / 402 error | Check your API key and account credits |
| No response / timeout | Increase `timeout_sec` in config, check internet connection |
| Response cut off | Increase `SS7AI_MAX_RESPONSE` define before include |
| Dialog doesn't open | Make sure `OnDialogResponse` calls `Lia_OnDialogResponse` |
| Termux: `ast.unparse` error | Upgrade Python: `pkg upgrade python` (needs 3.9+) |
| LIA responds out of context | Restart the bridge — stale `_res_*.json` files from previous session |

---

## 📁 Scriptfiles Reference

```
scriptfiles/
├── ss7ai_config.json    ← AI provider settings (edit this)
├── ss7ai_req_*.json     ← Temporary request files (auto-deleted)
├── ss7ai_res_*.json     ← Temporary response files (auto-deleted)
├── ss7ai_clear_*.json   ← Clear history signals (auto-deleted)
├── ss7ai.log            ← Bridge activity log
└── ss7ai.pid            ← Bridge process ID
```

---

### Using Xiaomi MiMo (free 2$ credits token)

<img src="https://i.imgur.com/9AUZb01.png" width="450" alt="MiMo Referral Code KURCZ8">

Use invite code **`KURCZ8`** when signing up for Xiaomi MiMo.  
Both you and the developer get **$2 in API credits** — valid for 40 days.

Configure `ss7ai_config.json`:
```json
{
    "provider": "Xiaomi MiMo",
    "api_key": "your-api-key-here",
    "api_url": "https://api.mimo.xiaomi.com/v1/chat/completions",
    "model": "mimo-v2-flash",
    "timeout_sec": 20
}
```

---

## 📸 Screenshots

<div align="center">
  <img src="https://i.imgur.com/NPmmrZl.png" width="45%" alt="Message">
  <br>
  <img src="https://i.imgur.com/Fh8c8oP.png" width="45%" alt="Dialog">
</div>

---

## 🤝 Credits

- **Developer:** Saints7
- **License:** MIT
- **Repository:** [github.com/Saints7-gh/ss7ai](https://github.com/Saints7-gh/ss7ai)
- **Framework:** Built for open.mp — also compatible with SA-MP

---

## 📜 License

MIT License — free to use, modify, and distribute.  
Credits appreciated but not required.

---

<div align="center">

Made with 🧠 by **Saints7**

*"You are not hard to manage. You just can't be managed by the wrong system."*

[Report Bug](https://github.com/Saints7-gh/ss7ai/issues) · [Request Feature](https://github.com/Saints7-gh/ss7ai/issues) ·

</div>
