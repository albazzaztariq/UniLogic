// extension.js — VS Code extension entry point for UniLogic language support
// Launches LSP/lsp.py as a language server over stdio.

const fs = require("fs");
const path = require("path");
const vscode = require("vscode");
const { LanguageClient, TransportKind } = require("vscode-languageclient/node");
const Anthropic = require("@anthropic-ai/sdk");

let client;
let explainDecorationType = null;

function findPython() {
    const candidates = [
        "C:\\Users\\azt12\\AppData\\Local\\Programs\\Python\\Python312\\python.exe",
        path.join(process.env.LOCALAPPDATA || "", "Programs", "Python", "Python312", "python.exe"),
        path.join(process.env.LOCALAPPDATA || "", "Programs", "Python", "Python313", "python.exe"),
        path.join(process.env.LOCALAPPDATA || "", "Programs", "Python", "Python311", "python.exe"),
        path.join(process.env.LOCALAPPDATA || "", "Programs", "Python", "Python310", "python.exe"),
    ];
    for (const p of candidates) {
        try { if (fs.existsSync(p)) return p; } catch { }
    }
    // Last resort — try py launcher which is always on PATH with Python for Windows
    try {
        require("child_process").execFileSync("py", ["--version"], { stdio: "ignore" });
        return "py";
    } catch { }
    return null;
}

function findMainPy(activeFilePath) {
    const config = vscode.workspace.getConfiguration("unilogic");

    // 1. Check unilogic.compilerPath setting
    const compilerPath = config.get("compilerPath", "");
    if (compilerPath) {
        const candidate = path.join(compilerPath, "Main.py");
        if (fs.existsSync(candidate)) return candidate;
    }

    // 2. Same directory as the open file
    if (activeFilePath) {
        const candidate = path.join(path.dirname(activeFilePath), "Main.py");
        if (fs.existsSync(candidate)) return candidate;
    }

    // 3. Workspace root folder
    const folders = vscode.workspace.workspaceFolders;
    if (folders) {
        const candidate = path.join(folders[0].uri.fsPath, "Main.py");
        if (fs.existsSync(candidate)) return candidate;
    }

    return null;
}

function activate(context) {
    // Workspace trust check — warn user if workspace is not trusted
    function checkTrust() {
        if (!vscode.workspace.isTrusted) {
            vscode.window.showErrorMessage(
                'UniLogic requires workspace trust to function. The extension is disabled until you enable it.',
                { modal: true },
                'Enable Trust'
            ).then(choice => {
                if (choice === 'Enable Trust') {
                    vscode.commands.executeCommand('workbench.action.manageTrustedDomain');
                }
            });
        }
    }

    checkTrust();
    vscode.workspace.onDidGrantWorkspaceTrust(() => {
        vscode.window.showInformationMessage('UniLogic is now active.');
    });

    // Welcome message on first install
    const hasShownWelcome = context.globalState.get('unilogic.welcomeShown2');
    if (!hasShownWelcome) {
        context.globalState.update('unilogic.welcomeShown2', true);
        vscode.window.showInformationMessage(
            'UniLogic: To use the code walkthrough feature, configure your AI provider in settings. Works with Ollama (free, local), OpenAI, Anthropic, or any OpenAI-compatible endpoint.',
            { modal: true },
            'Got it'
        );
    }

    const config = vscode.workspace.getConfiguration("unilogic");
    if (!config.get("lsp.enabled", true)) {
        console.log("UniLogic LSP disabled by setting.");
        return;
    }

    // Resolve lsp.py bundled with the extension
    const lspScript = path.join(__dirname, "lsp.py");

    // Which Python to use — user setting takes priority, then auto-detect
    let pythonPath = config.get("pythonPath", "");
    if (!pythonPath) {
        pythonPath = findPython();
    }
    if (!pythonPath) {
        vscode.window.showErrorMessage(
            'UniLogic: Python not found. Please locate your python.exe.',
            { modal: true },
            'Browse for Python'
        ).then(async choice => {
            if (choice === 'Browse for Python') {
                const result = await vscode.window.showOpenDialog({
                    canSelectFiles: true,
                    canSelectFolders: false,
                    filters: { 'Python': ['exe'] },
                    title: 'Select python.exe'
                });
                if (result && result[0]) {
                    const selectedPath = result[0].fsPath;
                    await vscode.workspace.getConfiguration().update(
                        'unilogic.pythonPath',
                        selectedPath,
                        vscode.ConfigurationTarget.Global
                    );
                    vscode.window.showInformationMessage(
                        'Python path saved. Please reload VS Code.',
                        'Reload'
                    ).then(c => {
                        if (c === 'Reload') vscode.commands.executeCommand('workbench.action.reloadWindow');
                    });
                }
            }
        });
        return;
    }

    // Auto-install pygls if missing
    const { execFileSync } = require('child_process');
    try {
        execFileSync(pythonPath, ['-c', 'import pygls'], { stdio: 'ignore' });
    } catch {
        // pygls not installed — install it silently
        vscode.window.withProgress({
            location: vscode.ProgressLocation.Notification,
            title: 'UniLogic: Installing language server dependencies...',
            cancellable: false
        }, () => new Promise((resolve, reject) => {
            const { exec } = require('child_process');
            exec(`"${pythonPath}" -m pip install pygls`, (err) => {
                if (err) {
                    vscode.window.showErrorMessage('UniLogic: Failed to install pygls. Run: pip install pygls');
                    reject(err);
                } else {
                    resolve();
                }
            });
        }));
    }

    const serverOptions = {
        command: pythonPath,
        args: [lspScript],
        transport: TransportKind.stdio,
    };

    const clientOptions = {
        documentSelector: [{ scheme: "file", language: "ul" }],
        synchronize: {
            fileEvents: vscode.workspace.createFileSystemWatcher("**/*.ul"),
        },
    };

    const outputChannel = vscode.window.createOutputChannel("UniLogic LSP");
    clientOptions.outputChannel = outputChannel;

    client = new LanguageClient(
        "unilogic",
        "UniLogic Language Server",
        serverOptions,
        clientOptions
    );

    client.start().then(() => {
        outputChannel.appendLine(`LSP started — python: ${pythonPath}, script: ${lspScript}`);
    }).catch(err => {
        outputChannel.appendLine(`LSP FAILED TO START: ${err.message}`);
        outputChannel.show(true);
        vscode.window.showErrorMessage(`UniLogic LSP failed: ${err.message}`);
    });
    console.log(`UniLogic LSP starting — python: ${pythonPath}, script: ${lspScript}`);

    // Fix language for already-open documents (including session-restored files)
    // Delay needed because session restore is async
    setTimeout(() => {
        vscode.workspace.textDocuments.forEach(doc => {
            if (doc.fileName.endsWith('.ul') && doc.languageId !== 'ul') {
                vscode.languages.setTextDocumentLanguage(doc, 'ul');
            }
        });
    }, 2000);

    // Apply to any .ul file opened during the session
    vscode.workspace.onDidOpenTextDocument(doc => {
        if (doc.fileName.endsWith('.ul') && doc.languageId !== 'ul') {
            vscode.languages.setTextDocumentLanguage(doc, 'ul');
        }
    });

    // Register commands
    context.subscriptions.push(
        vscode.commands.registerCommand("unilogic.explainStructure", () => explainFile(context, "structure")),
        vscode.commands.registerCommand("unilogic.explainExecution", () => explainFile(context, "execution")),
        vscode.commands.registerCommand("unilogic.testConnection", () => testAIConnection()),
        vscode.commands.registerCommand("unilogic.selectModel", () => selectAIModel()),
        vscode.commands.registerCommand("unilogic.showDependencyGraph", () => showDependencyGraph(context)),
        vscode.commands.registerCommand("unilogic.showSymbolOutline", () => showSymbolOutline(context)),
        vscode.commands.registerCommand("unilogic.showDrDiff", () => showDrDiff(context)),
        vscode.commands.registerCommand("unilogic.showCOutput", () => showCOutput(context)),
        vscode.commands.registerCommand("unilogic.runFile", () => runFile(context)),
        vscode.commands.registerCommand("unilogic.compileFile", () => compileFile(context)),
        vscode.commands.registerCommand("unilogic.showNormView", () => showNormView(context)),
        vscode.commands.registerCommand("unilogic.showProfile", () => showProfile(context)),
        vscode.commands.registerCommand("unilogic.runWithProfile", () => runWithProfile(context)),
        vscode.commands.registerCommand("unilogic.showDrMonitor", () => showDrMonitor(context)),
        vscode.commands.registerCommand("unilogic.applyHotPathHighlights", () => applyHotPathHighlights(context)),
        vscode.commands.registerCommand("unilogic.toggleFunctionLogging", () => toggleFunctionLogging(context)),
        vscode.commands.registerCommand("unilogic.formatDocument", () => {
            const editor = vscode.window.activeTextEditor;
            if (editor && editor.document.languageId === 'ul') {
                vscode.commands.executeCommand('editor.action.formatDocument');
            } else {
                vscode.window.showWarningMessage("Open a .ul file first.");
            }
        })
    );

    // Rotating tips in the status bar — only visible when a .ul file is active
    const TIPS = [
        "$(lightbulb) Ctrl+Click a function to jump to its definition",
        "$(book) Ctrl+Shift+P \u2192 UniLogic: Explain Structure",
        "$(play) Ctrl+Shift+P \u2192 UniLogic: Explain Execution Flow",
        "$(type-hierarchy) Ctrl+Shift+P \u2192 UniLogic: Show Dependency Graph",
        "$(search) Hover over any keyword for its definition",
        "$(beaker) Ctrl+Shift+P \u2192 UniLogic: Test AI Connection",
    ];

    const statusItem = vscode.window.createStatusBarItem(vscode.StatusBarAlignment.Right, 100);
    let tipIndex = 0;
    function rotateTip() {
        statusItem.text = TIPS[tipIndex % TIPS.length];
        tipIndex++;
    }
    rotateTip();
    setInterval(rotateTip, 20000);
    context.subscriptions.push(statusItem);

    // Show only when a .ul file is active
    if (vscode.window.activeTextEditor?.document.languageId === 'ul') {
        statusItem.show();
    }
    vscode.window.onDidChangeActiveTextEditor(editor => {
        if (editor?.document.languageId === 'ul') {
            statusItem.show();
        } else {
            statusItem.hide();
        }
    });

    // DR gutter annotations — apply diamond icons at DR boundary call sites
    const drWarningDecor = vscode.window.createTextEditorDecorationType({
        gutterIconPath: path.join(__dirname, 'media', 'dr-warning.svg'),
        gutterIconSize: '80%',
    });
    const drErrorDecor = vscode.window.createTextEditorDecorationType({
        gutterIconPath: path.join(__dirname, 'media', 'dr-error.svg'),
        gutterIconSize: '80%',
    });
    context.subscriptions.push(drWarningDecor, drErrorDecor);

    // Update DR gutter decorations when diagnostics change
    vscode.languages.onDidChangeDiagnostics(e => {
        for (const uri of e.uris) {
            const editor = vscode.window.visibleTextEditors.find(
                ed => ed.document.uri.toString() === uri.toString()
            );
            if (!editor || editor.document.languageId !== 'ul') continue;
            const diags = vscode.languages.getDiagnostics(uri);
            const warnings = [];
            const errors = [];
            for (const d of diags) {
                if (d.source === 'ul-dr-boundary') {
                    if (d.severity === vscode.DiagnosticSeverity.Warning) {
                        warnings.push({ range: d.range });
                    } else if (d.severity === vscode.DiagnosticSeverity.Error) {
                        errors.push({ range: d.range });
                    }
                }
            }
            editor.setDecorations(drWarningDecor, warnings);
            editor.setDecorations(drErrorDecor, errors);
        }
    });
}

// ---------------------------------------------------------------------------
// AI provider helpers — supports Anthropic and any OpenAI-compatible endpoint
// ---------------------------------------------------------------------------

function getAIConfig() {
    const config = vscode.workspace.getConfiguration("unilogic");
    return {
        provider: config.get("ai.provider", "openai-compatible"),
        apiKey:   config.get("ai.apiKey", ""),
        endpoint: config.get("ai.endpoint", "http://localhost:11434/v1"),
        model:    config.get("ai.model", "llama3"),
    };
}

const PROMPTS = {
    structure: `You are analyzing a UniLogic source file. Explain the structure only — what types are declared, what functions exist, what their signatures mean, what each parameter is. Do not explain execution or runtime behavior. Return a JSON array of steps: { "section": "name", "startLine": N, "endLine": N, "explanation": "plain English" }
Return ONLY the JSON array — no markdown fences, no extra text.`,

    execution: `You are analyzing a UniLogic source file. Explain execution flow only — starting from main(), what gets called, in what order, what data flows where, what each function does when it runs. Do not explain declarations or signatures. Return a JSON array of steps: { "section": "name", "startLine": N, "endLine": N, "explanation": "plain English" }
Return ONLY the JSON array — no markdown fences, no extra text.`,
};

async function callAI(aiCfg, systemPrompt, userContent) {
    if (aiCfg.provider === "anthropic") {
        const Anthropic = require("@anthropic-ai/sdk");
        const anthropic = new Anthropic({ apiKey: aiCfg.apiKey });
        const response = await anthropic.messages.create({
            model: aiCfg.model || "claude-sonnet-4-6",
            max_tokens: 4096,
            system: systemPrompt,
            messages: [{ role: "user", content: userContent }],
        });
        return response.content[0].text.trim();
    }

    // OpenAI-compatible (Ollama, OpenAI, LM Studio, etc.)
    const url = `${aiCfg.endpoint.replace(/\/+$/, "")}/chat/completions`;
    const headers = { "Content-Type": "application/json" };
    if (aiCfg.apiKey) headers["Authorization"] = `Bearer ${aiCfg.apiKey}`;

    const body = JSON.stringify({
        model: aiCfg.model,
        messages: [
            { role: "system", content: systemPrompt },
            { role: "user", content: userContent },
        ],
        temperature: 0.2,
    });

    const fetch = globalThis.fetch || require("node-fetch");
    const resp = await fetch(url, { method: "POST", headers, body });
    if (!resp.ok) {
        const errText = await resp.text();
        throw new Error(`${resp.status} ${resp.statusText}: ${errText}`);
    }
    const data = await resp.json();
    return data.choices[0].message.content.trim();
}

function parseAIResponse(text) {
    const cleaned = text.replace(/^```(?:json)?\s*/i, "").replace(/\s*```$/i, "");
    return JSON.parse(cleaned);
}

// ---------------------------------------------------------------------------
// Test AI Connection
// ---------------------------------------------------------------------------

function validateAISettings(aiCfg) {
    const warnings = [];
    const OLLAMA_DEFAULT = "http://localhost:11434/v1";

    if (!aiCfg.model) {
        warnings.push({ level: "error", msg: "Model name is required. Set unilogic.ai.model in settings." });
        return warnings; // fatal — no point continuing
    }

    if (aiCfg.provider === "anthropic") {
        if (!aiCfg.apiKey) {
            warnings.push({ level: "error", msg: "Anthropic requires an API key. Set unilogic.ai.apiKey in settings." });
        }
        if (aiCfg.endpoint && aiCfg.endpoint !== OLLAMA_DEFAULT) {
            warnings.push({ level: "warn", msg: "Endpoint is ignored when provider is set to Anthropic." });
        }
        if (!aiCfg.model.startsWith("claude-")) {
            warnings.push({ level: "warn", msg: `Model "${aiCfg.model}" doesn't look like an Anthropic model. Expected something like claude-sonnet-4-6.` });
        }
    }

    if (aiCfg.provider === "openai-compatible") {
        const isOllama = aiCfg.endpoint === OLLAMA_DEFAULT || !aiCfg.endpoint;
        if (!aiCfg.apiKey && isOllama) {
            warnings.push({ level: "info", msg: "Using Ollama locally. Make sure Ollama is running with your chosen model." });
        }
        if (aiCfg.apiKey && isOllama) {
            warnings.push({ level: "warn", msg: "You have an API key set but endpoint points to local Ollama. Did you mean to set a remote endpoint?" });
        }
    }

    return warnings;
}

async function testAIConnection() {
    const aiCfg = getAIConfig();

    // Validate settings first
    const issues = validateAISettings(aiCfg);
    let hasError = false;
    for (const issue of issues) {
        if (issue.level === "error") {
            vscode.window.showErrorMessage(`UniLogic: ${issue.msg}`);
            hasError = true;
        } else if (issue.level === "warn") {
            vscode.window.showWarningMessage(`UniLogic: ${issue.msg}`);
        } else {
            vscode.window.showInformationMessage(`UniLogic: ${issue.msg}`);
        }
    }
    if (hasError) return;

    await vscode.window.withProgress(
        { location: vscode.ProgressLocation.Notification, title: "UniLogic: Testing AI connection...", cancellable: false },
        async () => {
            try {
                if (aiCfg.provider === "anthropic") {
                    const Anthropic = require("@anthropic-ai/sdk");
                    const anthropic = new Anthropic({ apiKey: aiCfg.apiKey });
                    await anthropic.messages.create({
                        model: aiCfg.model,
                        max_tokens: 16,
                        messages: [{ role: "user", content: "Say OK" }],
                    });
                } else {
                    const url = `${aiCfg.endpoint.replace(/\/+$/, "")}/chat/completions`;
                    const headers = { "Content-Type": "application/json" };
                    if (aiCfg.apiKey) headers["Authorization"] = `Bearer ${aiCfg.apiKey}`;
                    const body = JSON.stringify({
                        model: aiCfg.model,
                        messages: [{ role: "user", content: "Say OK" }],
                        max_tokens: 16,
                    });
                    const fetch = globalThis.fetch || require("node-fetch");
                    const resp = await fetch(url, { method: "POST", headers, body });
                    if (!resp.ok) {
                        const errText = await resp.text();
                        throw new Error(`${resp.status}: ${errText}`);
                    }
                }
                vscode.window.showInformationMessage(`UniLogic: Connected — ${aiCfg.provider} / ${aiCfg.model}`);
            } catch (err) {
                vscode.window.showErrorMessage(`UniLogic: Connection failed — ${err.message}`);
            }
        }
    );
}

// ---------------------------------------------------------------------------
// Select AI Model — fetches available models and shows a QuickPick
// ---------------------------------------------------------------------------

const ANTHROPIC_MODELS = [
    { label: "claude-opus-4-6", description: "Most capable" },
    { label: "claude-sonnet-4-6", description: "Balanced" },
    { label: "claude-haiku-4-5-20251001", description: "Fast & cheap" },
];

async function selectAIModel() {
    const aiCfg = getAIConfig();
    let items = [];

    await vscode.window.withProgress(
        { location: vscode.ProgressLocation.Notification, title: "UniLogic: Fetching models...", cancellable: false },
        async () => {
            try {
                if (aiCfg.provider === "anthropic") {
                    // Try API fetch first
                    if (aiCfg.apiKey) {
                        try {
                            const Anthropic = require("@anthropic-ai/sdk");
                            const anthropic = new Anthropic({ apiKey: aiCfg.apiKey });
                            const list = await anthropic.models.list({ limit: 50 });
                            if (list.data && list.data.length > 0) {
                                items = list.data
                                    .filter(m => m.id.startsWith("claude-"))
                                    .map(m => ({ label: m.id, description: m.display_name || "" }));
                            }
                        } catch { /* fall through to hardcoded */ }
                    }
                    if (items.length === 0) items = ANTHROPIC_MODELS;
                } else {
                    // OpenAI-compatible — GET /models
                    const url = `${aiCfg.endpoint.replace(/\/+$/, "")}/models`;
                    const headers = {};
                    if (aiCfg.apiKey) headers["Authorization"] = `Bearer ${aiCfg.apiKey}`;
                    const fetch = globalThis.fetch || require("node-fetch");
                    const resp = await fetch(url, { headers });
                    if (resp.ok) {
                        const data = await resp.json();
                        const models = data.data || data.models || [];
                        items = models.map(m => ({
                            label: m.id || m.name,
                            description: m.owned_by || "",
                        }));
                        items.sort((a, b) => a.label.localeCompare(b.label));
                    }
                }
            } catch { /* ignore — will show free text fallback */ }
        }
    );

    let selected;
    if (items.length > 0) {
        // Mark the current model
        const currentModel = aiCfg.model;
        items.forEach(item => {
            if (item.label === currentModel) item.description = `${item.description} (current)`.trim();
        });
        selected = await vscode.window.showQuickPick(items, {
            placeHolder: `Current: ${aiCfg.model} — Select a model`,
            title: "UniLogic: Select AI Model",
        });
        if (selected) selected = selected.label;
    } else {
        // No models fetched — free text input
        selected = await vscode.window.showInputBox({
            prompt: "Enter model name (could not fetch model list from endpoint)",
            placeHolder: "e.g. llama3, mistral, gpt-4o",
            value: aiCfg.model,
            title: "UniLogic: Select AI Model",
        });
    }

    if (selected) {
        await vscode.workspace.getConfiguration().update(
            "unilogic.ai.model", selected, vscode.ConfigurationTarget.Global
        );
        vscode.window.showInformationMessage(`UniLogic: Model set to "${selected}"`);
    }
}

// ---------------------------------------------------------------------------
// Explain File — calls AI provider, shows walkthrough in a Webview
// mode: "structure" or "execution"
// ---------------------------------------------------------------------------

async function explainFile(context, mode) {
    const editor = vscode.window.activeTextEditor;
    if (!editor) {
        vscode.window.showWarningMessage("Open a .ul file first.");
        return;
    }
    if (!editor.document.fileName.endsWith(".ul")) {
        vscode.window.showWarningMessage("This command only works on .ul files.");
        return;
    }

    const aiCfg = getAIConfig();

    if (aiCfg.provider === "anthropic" && !aiCfg.model.startsWith("claude-")) {
        const choice = await vscode.window.showWarningMessage(
            `Model '${aiCfg.model}' doesn't look right for Anthropic. Run 'UniLogic: Select AI Model' to pick one.`,
            "Select Model"
        );
        if (choice === "Select Model") {
            vscode.commands.executeCommand("unilogic.selectModel");
        }
        return;
    }

    if (aiCfg.provider === "anthropic" && !aiCfg.apiKey) {
        vscode.window.showWarningMessage(
            "UniLogic: No API key configured. Set unilogic.ai.apiKey in settings, or switch to an OpenAI-compatible provider (e.g. Ollama) which works without a key.",
            "Open Settings"
        ).then(choice => {
            if (choice === "Open Settings") {
                vscode.commands.executeCommand("workbench.action.openSettings", "unilogic.ai");
            }
        });
        return;
    }

    const fileContent = editor.document.getText();
    const fileName = path.basename(editor.document.fileName);

    const modeLabel = mode === "structure" ? "structure" : "execution flow";
    await vscode.window.withProgress(
        { location: vscode.ProgressLocation.Notification, title: `Explaining ${modeLabel}...`, cancellable: false },
        async () => {
            let steps;
            try {
                const raw = await callAI(aiCfg, PROMPTS[mode], `File: ${fileName}\n\n${fileContent}`);
                steps = parseAIResponse(raw);
            } catch (err) {
                vscode.window.showErrorMessage(`UniLogic AI error: ${err.message}`);
                return;
            }

            if (!steps || steps.length === 0) {
                vscode.window.showWarningMessage("No explanation steps returned.");
                return;
            }

            showExplainPanel(context, editor, steps, fileName);
        }
    );
}

function showExplainPanel(context, editor, steps, fileName) {
    // Create decoration type for line highlighting
    if (explainDecorationType) {
        explainDecorationType.dispose();
    }
    explainDecorationType = vscode.window.createTextEditorDecorationType({
        backgroundColor: "rgba(255, 213, 79, 0.18)",
        isWholeLine: true,
        overviewRulerColor: "rgba(255, 213, 79, 0.7)",
        overviewRulerLane: vscode.OverviewRulerLane.Center,
    });

    let currentStep = 0;

    const panel = vscode.window.createWebviewPanel(
        "unilogicExplain",
        `Explain: ${fileName}`,
        vscode.ViewColumn.Beside,
        { enableScripts: true }
    );

    function updatePanel() {
        panel.webview.html = getWebviewHTML(steps, currentStep, fileName);
        highlightLines(editor, steps[currentStep]);
    }

    panel.webview.onDidReceiveMessage(msg => {
        if (msg.command === "prev" && currentStep > 0) {
            currentStep--;
            updatePanel();
        } else if (msg.command === "next" && currentStep < steps.length - 1) {
            currentStep++;
            updatePanel();
        } else if (msg.command === "goto" && typeof msg.index === "number") {
            currentStep = Math.max(0, Math.min(msg.index, steps.length - 1));
            updatePanel();
        }
    });

    panel.onDidDispose(() => {
        if (explainDecorationType) {
            // Clear decorations from all visible editors
            vscode.window.visibleTextEditors.forEach(ed => {
                ed.setDecorations(explainDecorationType, []);
            });
        }
    });

    updatePanel();
}

function highlightLines(editor, step) {
    if (!explainDecorationType || !step) return;
    const startLine = Math.max(0, (step.startLine || 1) - 1);
    const endLine = Math.max(startLine, (step.endLine || step.startLine || 1) - 1);
    const range = new vscode.Range(startLine, 0, endLine, Number.MAX_SAFE_INTEGER);
    editor.setDecorations(explainDecorationType, [range]);
    editor.revealRange(range, vscode.TextEditorRevealType.InCenterIfOutsideViewport);
}

function getWebviewHTML(steps, currentIndex, fileName) {
    const step = steps[currentIndex];
    const stepsListHTML = steps.map((s, i) => {
        const active = i === currentIndex ? ' class="active"' : "";
        return `<li${active}><button onclick="goto(${i})">${escapeHtml(s.section)}</button></li>`;
    }).join("\n");

    return `<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="UTF-8"/>
<meta name="viewport" content="width=device-width, initial-scale=1.0"/>
<style>
  body {
    font-family: var(--vscode-font-family, sans-serif);
    color: var(--vscode-foreground);
    background: var(--vscode-editor-background);
    padding: 16px;
    margin: 0;
  }
  h2 { margin: 0 0 4px 0; font-size: 1.15em; }
  .meta { opacity: 0.6; font-size: 0.85em; margin-bottom: 12px; }
  .explanation {
    background: var(--vscode-textBlockQuote-background, rgba(255,255,255,0.05));
    border-left: 3px solid var(--vscode-textLink-foreground, #4fc1ff);
    padding: 12px 16px;
    margin: 12px 0;
    line-height: 1.5;
    font-size: 0.95em;
  }
  .nav { display: flex; gap: 8px; margin: 16px 0; }
  .nav button {
    padding: 6px 16px;
    cursor: pointer;
    background: var(--vscode-button-background);
    color: var(--vscode-button-foreground);
    border: none;
    border-radius: 3px;
  }
  .nav button:disabled { opacity: 0.4; cursor: default; }
  .nav button:hover:not(:disabled) { background: var(--vscode-button-hoverBackground); }
  ul { list-style: none; padding: 0; margin: 16px 0 0 0; }
  li { margin: 2px 0; }
  li button {
    background: none;
    border: none;
    color: var(--vscode-textLink-foreground, #4fc1ff);
    cursor: pointer;
    text-align: left;
    padding: 2px 4px;
    font-size: 0.9em;
  }
  li button:hover { text-decoration: underline; }
  li.active button { font-weight: bold; text-decoration: underline; }
  .counter { opacity: 0.6; font-size: 0.85em; }
</style>
</head>
<body>
  <h2>${escapeHtml(step.section)}</h2>
  <div class="meta">Lines ${step.startLine}–${step.endLine} &nbsp;·&nbsp; ${fileName}</div>
  <div class="explanation">${escapeHtml(step.explanation)}</div>
  <div class="nav">
    <button onclick="prev()" ${currentIndex === 0 ? "disabled" : ""}>← Previous</button>
    <span class="counter">${currentIndex + 1} / ${steps.length}</span>
    <button onclick="next()" ${currentIndex === steps.length - 1 ? "disabled" : ""}>Next →</button>
  </div>
  <h3>Sections</h3>
  <ul>${stepsListHTML}</ul>
  <script>
    const vscode = acquireVsCodeApi();
    function prev() { vscode.postMessage({ command: "prev" }); }
    function next() { vscode.postMessage({ command: "next" }); }
    function goto(i) { vscode.postMessage({ command: "goto", index: i }); }
  </script>
</body>
</html>`;
}

function escapeHtml(str) {
    return String(str)
        .replace(/&/g, "&amp;")
        .replace(/</g, "&lt;")
        .replace(/>/g, "&gt;")
        .replace(/"/g, "&quot;");
}

// ---------------------------------------------------------------------------
// Dependency Graph — scans .ul files, builds import/call graph, shows in webview
// ---------------------------------------------------------------------------

async function showDependencyGraph(context) {
    const folders = vscode.workspace.workspaceFolders;
    if (!folders) {
        vscode.window.showErrorMessage("UniLogic: Open a workspace folder first.");
        return;
    }

    // Find all .ul files in the workspace
    const files = await vscode.workspace.findFiles("**/*.ul", "**/node_modules/**");
    if (files.length === 0) {
        vscode.window.showInformationMessage("UniLogic: No .ul files found in workspace.");
        return;
    }

    // Parse each file for: function declarations, imports, function calls, DR settings
    const nodes = [];   // { id, label, path, drMemory }
    const edges = [];   // { source, target }
    const funcToFiles = {};   // function name → Set of file ids that declare it
    const fileDeclaredFuncs = {};  // file id → Set of function names declared in it
    const seenIds = new Set();  // dedup guard

    // UL keywords that look like calls but aren't (e.g. "if (", "while (")
    const KEYWORD_SKIP = new Set([
        "function","end","returns","return","if","else","while","do","for","each",
        "in","match","iterate","default","escape","continue","type","inherits",
        "new","fixed","constant","import","from","export","print","prompt",
        "parallel","killswitch","teleport","portal","nocache","yield","inline",
        "pack","true","false","empty","and","or","not","equals","address","deref",
        "memmove","memcopy","memset","memtake","memgive","size","change","absval",
        "int","integer","float","double","string","bool","none","complex",
        "int8","int16","int32","int64","uint8","uint16","uint32","uint64",
        "array","list","map","arena","file","ok","error","some",
    ]);

    for (const uri of files) {
        const text = (await vscode.workspace.fs.readFile(uri)).toString();
        const relPath = vscode.workspace.asRelativePath(uri);
        const fileId = relPath.replace(/\\/g, "/");

        // Deduplicate: skip if we already have a node with this id
        if (seenIds.has(fileId)) continue;
        seenIds.add(fileId);

        // Extract DR memory setting from comments like: memory: gc
        let drMemory = "none";
        const drMatch = text.match(/memory\s*:\s*(gc|manual|refcount)/i);
        if (drMatch) drMemory = drMatch[1].toLowerCase();

        nodes.push({ id: fileId, label: path.basename(uri.fsPath), path: uri.fsPath, drMemory });

        // Extract function declarations
        const declaredHere = new Set();
        const funcDeclRegex = /^function\s+(\w+)\s*\(/gm;
        let m;
        while ((m = funcDeclRegex.exec(text)) !== null) {
            const name = m[1];
            declaredHere.add(name);
            if (!funcToFiles[name]) funcToFiles[name] = new Set();
            funcToFiles[name].add(fileId);
        }
        fileDeclaredFuncs[fileId] = declaredHere;
    }

    // Build set of functions that are uniquely declared in exactly one file
    // (functions like "main" that exist in many files create no cross-file edges)
    const uniqueFuncs = {};  // name → fileId (only if declared in exactly one file)
    for (const [name, fileSet] of Object.entries(funcToFiles)) {
        if (fileSet.size === 1) {
            uniqueFuncs[name] = [...fileSet][0];
        }
    }

    // Second pass: find cross-file function calls
    for (const uri of files) {
        const text = (await vscode.workspace.fs.readFile(uri)).toString();
        const relPath = vscode.workspace.asRelativePath(uri);
        const sourceId = relPath.replace(/\\/g, "/");
        if (!seenIds.has(sourceId)) continue;  // skip dupes

        const localDecls = fileDeclaredFuncs[sourceId] || new Set();

        // Find function calls: name( but NOT preceded by "function " (which is a declaration)
        const callRegex = /(?<!function\s)(\b\w+)\s*\(/g;
        let m;
        const edgeTargets = new Set();
        while ((m = callRegex.exec(text)) !== null) {
            const name = m[1];
            // Skip: keywords, locally-declared functions, non-unique functions
            if (KEYWORD_SKIP.has(name)) continue;
            if (localDecls.has(name)) continue;
            if (!uniqueFuncs[name]) continue;
            const targetFile = uniqueFuncs[name];
            if (targetFile === sourceId) continue;
            if (edgeTargets.has(targetFile)) continue;
            edgeTargets.add(targetFile);
            edges.push({ source: sourceId, target: targetFile });
        }

        // Find import statements: import "libname" function name(...)
        const importRegex = /import\s+"([^"]+)"\s+function\s+(\w+)/g;
        while ((m = importRegex.exec(text)) !== null) {
            const lib = m[1];
            // If the import references a .ul file in the workspace
            const libFile = nodes.find(n => n.label === lib || n.label === lib + ".ul");
            if (libFile && libFile.id !== sourceId && !edgeTargets.has(libFile.id)) {
                edgeTargets.add(libFile.id);
                edges.push({ source: sourceId, target: libFile.id });
            }
        }
    }

    // Detect DR setting conflicts between connected files
    const conflicts = [];
    for (const edge of edges) {
        const srcNode = nodes.find(n => n.id === edge.source);
        const tgtNode = nodes.find(n => n.id === edge.target);
        if (srcNode && tgtNode && srcNode.drMemory !== "none" && tgtNode.drMemory !== "none"
            && srcNode.drMemory !== tgtNode.drMemory) {
            conflicts.push({ source: edge.source, target: edge.target });
        }
    }

    // Create webview panel
    const panel = vscode.window.createWebviewPanel(
        "unilogicDepGraph",
        "UniLogic: Dependency Graph",
        vscode.ViewColumn.One,
        { enableScripts: true }
    );

    panel.webview.html = getDependencyGraphHTML(nodes, edges, conflicts);

    // Handle click-to-open messages from the webview
    panel.webview.onDidReceiveMessage(msg => {
        if (msg.command === "openFile" && msg.path) {
            const uri = vscode.Uri.file(msg.path);
            vscode.window.showTextDocument(uri);
        }
    });
}

function getDependencyGraphHTML(nodes, edges, conflicts) {
    const nodesJSON = JSON.stringify(nodes);
    const edgesJSON = JSON.stringify(edges);
    const conflictsJSON = JSON.stringify(conflicts);

    return `<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1.0">
<title>UniLogic Dependency Graph</title>
<style>
body { margin: 0; overflow: hidden; background: #0a0a0f; font-family: 'Segoe UI', sans-serif; }
svg { width: 100vw; height: 100vh; }
.node circle { cursor: pointer; stroke-width: 2; }
.node text { fill: #e0e0e8; font-size: 12px; pointer-events: none; text-anchor: middle; }
.link { stroke-opacity: 0.6; fill: none; }
.link-normal { stroke: #4a5568; }
.link-conflict { stroke: #f38ba8; stroke-width: 2.5; stroke-dasharray: 6,3; }
.legend { position: fixed; bottom: 16px; left: 16px; background: rgba(30,30,46,0.92); border: 1px solid #313244; border-radius: 6px; padding: 12px 16px; color: #a6adc8; font-size: 12px; }
.legend-item { display: flex; align-items: center; gap: 8px; margin: 4px 0; }
.legend-dot { width: 12px; height: 12px; border-radius: 50%; flex-shrink: 0; }
.title-bar { position: fixed; top: 0; left: 0; right: 0; background: rgba(30,30,46,0.95); border-bottom: 1px solid #313244; padding: 10px 20px; color: #7ab3e0; font-size: 14px; font-weight: 600; letter-spacing: 0.05em; z-index: 10; }
marker { overflow: visible; }
</style>
</head>
<body>
<div class="title-bar">UniLogic Dependency Graph &mdash; ${nodes.length} files, ${edges.length} dependencies</div>
<svg id="graph"></svg>
<div class="legend">
  <div class="legend-item"><span class="legend-dot" style="background:#4a9ad0"></span> gc</div>
  <div class="legend-item"><span class="legend-dot" style="background:#a6e3a1"></span> manual</div>
  <div class="legend-item"><span class="legend-dot" style="background:#fab387"></span> refcount</div>
  <div class="legend-item"><span class="legend-dot" style="background:#585b70"></span> none / unset</div>
  <div class="legend-item" style="margin-top:8px;"><span style="width:20px;height:2px;background:#f38ba8;display:inline-block;"></span> DR conflict</div>
</div>
<script src="https://d3js.org/d3.v7.min.js"></script>
<script>
const vscode = acquireVsCodeApi();
const nodes = ${nodesJSON};
const links = ${edgesJSON};
const conflicts = ${conflictsJSON};

const conflictSet = new Set(conflicts.map(c => c.source + "|" + c.target));

const colorMap = { gc: "#4a9ad0", manual: "#a6e3a1", refcount: "#fab387", none: "#585b70" };

const svg = d3.select("#graph");
const width = window.innerWidth;
const height = window.innerHeight;

// Arrow marker
svg.append("defs").append("marker")
  .attr("id", "arrow").attr("viewBox", "0 -5 10 10")
  .attr("refX", 22).attr("refY", 0)
  .attr("markerWidth", 8).attr("markerHeight", 8)
  .attr("orient", "auto")
  .append("path").attr("d", "M0,-4L10,0L0,4").attr("fill", "#4a5568");

svg.append("defs").append("marker")
  .attr("id", "arrow-conflict").attr("viewBox", "0 -5 10 10")
  .attr("refX", 22).attr("refY", 0)
  .attr("markerWidth", 8).attr("markerHeight", 8)
  .attr("orient", "auto")
  .append("path").attr("d", "M0,-4L10,0L0,4").attr("fill", "#f38ba8");

const simulation = d3.forceSimulation(nodes)
  .force("link", d3.forceLink(links).id(d => d.id).distance(140))
  .force("charge", d3.forceManyBody().strength(-400))
  .force("center", d3.forceCenter(width / 2, height / 2))
  .force("collide", d3.forceCollide(50));

const link = svg.append("g").selectAll("line")
  .data(links).join("line")
  .attr("class", d => conflictSet.has(d.source.id + "|" + d.target.id) ? "link link-conflict" : "link link-normal")
  .attr("marker-end", d => conflictSet.has(d.source.id + "|" + d.target.id) ? "url(#arrow-conflict)" : "url(#arrow)");

const node = svg.append("g").selectAll("g")
  .data(nodes).join("g")
  .attr("class", "node")
  .call(d3.drag()
    .on("start", (e, d) => { if (!e.active) simulation.alphaTarget(0.3).restart(); d.fx = d.x; d.fy = d.y; })
    .on("drag", (e, d) => { d.fx = e.x; d.fy = e.y; })
    .on("end", (e, d) => { if (!e.active) simulation.alphaTarget(0); d.fx = null; d.fy = null; }));

node.append("circle")
  .attr("r", 16)
  .attr("fill", d => colorMap[d.drMemory] || colorMap.none)
  .attr("stroke", d => d3.color(colorMap[d.drMemory] || colorMap.none).brighter(0.5))
  .on("click", (e, d) => { vscode.postMessage({ command: "openFile", path: d.path }); });

node.append("text").attr("dy", 28).text(d => d.label);

simulation.on("tick", () => {
  link.attr("x1", d => d.source.x).attr("y1", d => d.source.y)
      .attr("x2", d => d.target.x).attr("y2", d => d.target.y);
  node.attr("transform", d => "translate(" + d.x + "," + d.y + ")");
});
</script>
</body>
</html>`;
}

// ---------------------------------------------------------------------------
// Symbol Outline — parses .ul file and shows structured outline in a webview
// ---------------------------------------------------------------------------

function showSymbolOutline(context) {
    const editor = vscode.window.activeTextEditor;
    if (!editor || !editor.document.fileName.endsWith('.ul')) {
        vscode.window.showWarningMessage("Open a .ul file first.");
        return;
    }

    const text = editor.document.getText();
    const fileName = path.basename(editor.document.fileName);
    const filePath = editor.document.uri;

    // Parse symbols from source text
    const drDirectives = [];
    const types = [];
    const functions = [];
    const imports = [];

    const lines = text.split('\n');
    for (let i = 0; i < lines.length; i++) {
        const line = lines[i];
        const trimmed = line.trim();
        const lineNum = i + 1;

        // @dr key = value
        const drMatch = trimmed.match(/^@dr\s+(\w+)\s*=\s*(\w+)/);
        if (drMatch) {
            drDirectives.push({ key: drMatch[1], value: drMatch[2], line: lineNum });
            continue;
        }

        // @norm N
        const normMatch = trimmed.match(/^@norm\s+(\d+)/);
        if (normMatch) {
            drDirectives.push({ key: "norm", value: normMatch[1], line: lineNum });
            continue;
        }

        // type Name ... end type  (collect fields from following lines)
        const typeMatch = trimmed.match(/^type\s+(\w+)(?:\s+inherits\s+(\w+))?/);
        if (typeMatch) {
            const fields = [];
            for (let j = i + 1; j < lines.length; j++) {
                const fl = lines[j].trim();
                if (fl.startsWith('end type')) break;
                const fm = fl.match(/^(\w+)\s+(\w+)/);
                if (fm) fields.push({ type: fm[1], name: fm[2] });
            }
            types.push({
                name: typeMatch[1],
                parent: typeMatch[2] || null,
                fields,
                line: lineNum,
            });
            continue;
        }

        // import "lib" function name(params) returns type
        const impMatch = trimmed.match(/^import\s+"([^"]+)"\s+function\s+(\w+)\s*\(([^)]*)\)(?:\s+returns\s+(\w+))?/);
        if (impMatch) {
            imports.push({
                lib: impMatch[1],
                name: impMatch[2],
                params: impMatch[3].trim(),
                returns: impMatch[4] || "none",
                line: lineNum,
            });
            continue;
        }

        // function name(params) returns type
        const fnMatch = trimmed.match(/^function\s+(\w+)\s*\(([^)]*)\)(?:\s+(?:returns|yields)\s+(\w+))?/);
        if (fnMatch) {
            functions.push({
                name: fnMatch[1],
                params: fnMatch[2].trim(),
                returns: fnMatch[3] || "none",
                line: lineNum,
            });
            continue;
        }
    }

    const panel = vscode.window.createWebviewPanel(
        "unilogicOutline",
        `Outline: ${fileName}`,
        vscode.ViewColumn.Beside,
        { enableScripts: true }
    );

    panel.webview.html = getSymbolOutlineHTML(fileName, drDirectives, types, functions, imports);

    panel.webview.onDidReceiveMessage(msg => {
        if (msg.command === "gotoLine" && typeof msg.line === "number") {
            const pos = new vscode.Position(msg.line - 1, 0);
            const range = new vscode.Range(pos, pos);
            vscode.window.showTextDocument(editor.document, { selection: range, viewColumn: vscode.ViewColumn.One });
        }
    });
}

function getSymbolOutlineHTML(fileName, drDirectives, types, functions, imports) {
    function renderSection(title, icon, items, renderItem) {
        if (items.length === 0) return '';
        const itemsHTML = items.map(renderItem).join('\n');
        return `<div class="section">
            <h2>${icon} ${escapeHtml(title)} <span class="count">(${items.length})</span></h2>
            <ul>${itemsHTML}</ul>
        </div>`;
    }

    const drHTML = renderSection("Directives", "\u2699", drDirectives, d =>
        `<li><button onclick="go(${d.line})"><span class="sym-key">@${escapeHtml(d.key)}</span> = <span class="sym-val">${escapeHtml(d.value)}</span></button><span class="line-num">:${d.line}</span></li>`
    );

    const typesHTML = renderSection("Types", "\u25A0", types, t => {
        const fieldsStr = t.fields.map(f => `<span class="field">${escapeHtml(f.type)} ${escapeHtml(f.name)}</span>`).join(', ');
        const inheritsStr = t.parent ? ` <span class="inherits">inherits ${escapeHtml(t.parent)}</span>` : '';
        return `<li><button onclick="go(${t.line})"><span class="sym-name">${escapeHtml(t.name)}</span>${inheritsStr}</button><span class="line-num">:${t.line}</span>
            ${fieldsStr ? `<div class="fields">${fieldsStr}</div>` : ''}</li>`;
    });

    const funcsHTML = renderSection("Functions", "\u25B6", functions, f =>
        `<li><button onclick="go(${f.line})"><span class="sym-name">${escapeHtml(f.name)}</span><span class="params">(${escapeHtml(f.params)})</span> <span class="returns">returns ${escapeHtml(f.returns)}</span></button><span class="line-num">:${f.line}</span></li>`
    );

    const importsHTML = renderSection("Foreign Imports", "\u2B95", imports, imp =>
        `<li><button onclick="go(${imp.line})"><span class="sym-name">${escapeHtml(imp.name)}</span><span class="params">(${escapeHtml(imp.params)})</span> <span class="returns">returns ${escapeHtml(imp.returns)}</span> <span class="lib">from "${escapeHtml(imp.lib)}"</span></button><span class="line-num">:${imp.line}</span></li>`
    );

    return `<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1.0">
<style>
body {
    font-family: var(--vscode-font-family, 'Segoe UI', sans-serif);
    color: var(--vscode-foreground);
    background: var(--vscode-editor-background);
    padding: 16px; margin: 0;
}
h1 { font-size: 1.1em; margin: 0 0 16px 0; opacity: 0.7; }
h2 { font-size: 1em; margin: 16px 0 6px 0; color: var(--vscode-textLink-foreground, #4fc1ff); }
.count { opacity: 0.5; font-weight: normal; }
.section { margin-bottom: 12px; }
ul { list-style: none; padding: 0; margin: 0; }
li { margin: 3px 0; display: flex; align-items: baseline; gap: 6px; }
li button {
    background: none; border: none; cursor: pointer; text-align: left;
    color: var(--vscode-foreground); font-size: 0.9em; padding: 2px 4px;
}
li button:hover { color: var(--vscode-textLink-foreground, #4fc1ff); text-decoration: underline; }
.sym-name { font-weight: 600; }
.sym-key { color: var(--vscode-textLink-foreground, #4fc1ff); }
.sym-val { color: var(--vscode-debugTokenExpression-string, #a6e3a1); }
.params { opacity: 0.7; }
.returns { opacity: 0.5; font-style: italic; }
.lib { opacity: 0.5; }
.inherits { opacity: 0.6; font-style: italic; }
.line-num { opacity: 0.35; font-size: 0.8em; flex-shrink: 0; }
.fields { margin-left: 24px; opacity: 0.6; font-size: 0.85em; }
.field { margin-right: 8px; }
</style>
</head>
<body>
<h1>${escapeHtml(fileName)}</h1>
${drHTML}${typesHTML}${funcsHTML}${importsHTML}
<script>
const vscode = acquireVsCodeApi();
function go(line) { vscode.postMessage({ command: "gotoLine", line }); }
</script>
</body>
</html>`;
}

// ---------------------------------------------------------------------------
// DR Diff — shows DR settings across all .ul files in a table
// ---------------------------------------------------------------------------

async function showDrDiff(context) {
    const files = await vscode.workspace.findFiles("**/*.ul", "**/node_modules/**");
    if (files.length === 0) {
        vscode.window.showInformationMessage("UniLogic: No .ul files found in workspace.");
        return;
    }

    const drKeys = new Set();
    const fileData = [];

    for (const uri of files) {
        const text = (await vscode.workspace.fs.readFile(uri)).toString();
        const relPath = vscode.workspace.asRelativePath(uri);
        const settings = {};

        // Parse @dr key = value
        const drRegex = /^@dr\s+(\w+)\s*=\s*(\w+)/gm;
        let m;
        while ((m = drRegex.exec(text)) !== null) {
            settings[m[1]] = m[2];
            drKeys.add(m[1]);
        }

        // Legacy comment: memory: gc
        if (!settings.memory) {
            const legacyMatch = text.match(/memory\s*:\s*(gc|manual|refcount)/i);
            if (legacyMatch) {
                settings.memory = legacyMatch[1].toLowerCase();
                drKeys.add("memory");
            }
        }

        fileData.push({ path: uri.fsPath, rel: relPath, settings });
    }

    const sortedKeys = [...drKeys].sort();

    // Find conflicts: keys where not all files have the same value
    const conflicts = new Set();
    for (const key of sortedKeys) {
        const values = new Set(fileData.map(f => f.settings[key] || "—"));
        if (values.size > 1) conflicts.add(key);
    }

    const panel = vscode.window.createWebviewPanel(
        "unilogicDrDiff", "UniLogic: DR Diff", vscode.ViewColumn.One, { enableScripts: true }
    );

    const headerCells = sortedKeys.map(k => {
        const cls = conflicts.has(k) ? ' class="conflict-key"' : '';
        return `<th${cls}>${escapeHtml(k)}</th>`;
    }).join('');

    const rows = fileData.map(f => {
        const cells = sortedKeys.map(k => {
            const val = f.settings[k] || "\u2014";
            const cls = conflicts.has(k) && val !== "\u2014" ? ' class="conflict"' : '';
            return `<td${cls}>${escapeHtml(val)}</td>`;
        }).join('');
        const hasConflict = sortedKeys.some(k => conflicts.has(k) && f.settings[k]);
        const rowCls = hasConflict ? ' class="conflict-row"' : '';
        return `<tr${rowCls}><td><button onclick="open('${f.path.replace(/\\/g, '\\\\').replace(/'/g, "\\'")}')">${escapeHtml(f.rel)}</button></td>${cells}</tr>`;
    }).join('\n');

    panel.webview.html = `<!DOCTYPE html>
<html><head><meta charset="UTF-8"><style>
body { font-family: var(--vscode-font-family, sans-serif); color: var(--vscode-foreground); background: var(--vscode-editor-background); padding: 16px; }
h1 { font-size: 1.1em; opacity: 0.7; margin-bottom: 12px; }
table { border-collapse: collapse; width: 100%; font-size: 0.9em; }
th, td { padding: 6px 12px; border: 1px solid var(--vscode-panel-border, #333); text-align: left; }
th { background: rgba(255,255,255,0.05); }
.conflict-key { color: #f38ba8; }
.conflict { color: #f38ba8; font-weight: 600; }
.conflict-row { background: rgba(243, 139, 168, 0.08); }
button { background: none; border: none; color: var(--vscode-textLink-foreground, #4fc1ff); cursor: pointer; text-align: left; padding: 0; }
button:hover { text-decoration: underline; }
.summary { margin-top: 12px; opacity: 0.6; font-size: 0.85em; }
</style></head><body>
<h1>DR Settings — ${fileData.length} files, ${sortedKeys.length} keys</h1>
${sortedKeys.length === 0 ? '<p>No @dr directives found in any .ul file.</p>' : `
<table><thead><tr><th>File</th>${headerCells}</tr></thead><tbody>${rows}</tbody></table>
<div class="summary">${conflicts.size > 0 ? `${conflicts.size} conflicting key(s): ${[...conflicts].join(', ')}` : 'No conflicts.'}</div>`}
<script>
const vscode = acquireVsCodeApi();
function open(p) { vscode.postMessage({ command: "openFile", path: p }); }
</script></body></html>`;

    panel.webview.onDidReceiveMessage(msg => {
        if (msg.command === "openFile" && msg.path) {
            vscode.window.showTextDocument(vscode.Uri.file(msg.path));
        }
    });
}

// ---------------------------------------------------------------------------
// Show C Output — transpiles .ul to C and opens diff view
// ---------------------------------------------------------------------------

let cOutputWatcher = null;

async function showCOutput(context) {
    const editor = vscode.window.activeTextEditor;
    if (!editor || !editor.document.fileName.endsWith('.ul')) {
        vscode.window.showWarningMessage("Open a .ul file first.");
        return;
    }

    const config = vscode.workspace.getConfiguration("unilogic");
    let pythonPath = config.get("pythonPath", "");
    if (!pythonPath) pythonPath = findPython();
    if (!pythonPath) {
        vscode.window.showErrorMessage("UniLogic: Python not found.");
        return;
    }

    const ulPath = editor.document.fileName;
    const tmpDir = require('os').tmpdir();
    const cPath = path.join(tmpDir, path.basename(ulPath, '.ul') + '.c');

    // Build the compile script
    const compileScript = `
import sys, os
_base = os.path.dirname(os.path.abspath(__file__))
# Bundled layout: compiler/ next to this script
_bundled = os.path.join(_base, "compiler")
if os.path.isdir(_bundled):
    sys.path.insert(0, _bundled)
else:
    # Dev layout: sibling dirs
    _root = os.path.dirname(_base)
    for sub in ("Lexer", "Parser", "AST", "Semantic", "Compiler", "Codegen"):
        sys.path.insert(0, os.path.join(_root, sub))
from lexer import tokenize
from parser import parse
from normalize import normalize
from semcheck import check
from codegen_c import generate
source = open(sys.argv[1], encoding='utf-8').read()
tokens = tokenize(source, sys.argv[1])
program = parse(tokens, sys.argv[1])
program = normalize(program, source=source)
check(program, sys.argv[1])
c_code = generate(program)
open(sys.argv[2], 'w', encoding='utf-8').write(c_code)
`;

    const compileScriptPath = path.join(context.extensionPath, '_compile_c.py');
    fs.writeFileSync(compileScriptPath, compileScript);

    async function compileAndShow() {
        try {
            const { execFileSync } = require('child_process');
            execFileSync(pythonPath, [compileScriptPath, ulPath, cPath], {
                cwd: context.extensionPath,
                stdio: ['pipe', 'pipe', 'pipe'],
                timeout: 15000,
            });
        } catch (err) {
            const stderr = err.stderr ? err.stderr.toString().trim() : '';
            const stdout = err.stdout ? err.stdout.toString().trim() : '';
            const detail = stderr || stdout || err.message;
            // Show the last meaningful line as the notification, full output to console
            const lines = detail.split('\n').filter(l => l.trim());
            const summary = lines[lines.length - 1] || detail;
            console.error('UniLogic C compile full output:', detail);
            vscode.window.showErrorMessage(`UniLogic C compile error: ${summary}`, { modal: false });
            // Also show in output channel for full traceback
            const chan = vscode.window.createOutputChannel("UniLogic Compiler");
            chan.clear();
            chan.appendLine(detail);
            chan.show(true);
            return;
        }

        const ulUri = vscode.Uri.file(ulPath);
        const cUri = vscode.Uri.file(cPath);
        await vscode.commands.executeCommand('vscode.diff', ulUri, cUri,
            `${path.basename(ulPath)} \u2192 ${path.basename(cPath)}`);
    }

    await compileAndShow();

    // Watch for saves and recompile
    if (cOutputWatcher) cOutputWatcher.dispose();
    cOutputWatcher = vscode.workspace.onDidSaveTextDocument(doc => {
        if (doc.fileName === ulPath) {
            compileAndShow();
        }
    });
    context.subscriptions.push(cOutputWatcher);
}

// ---------------------------------------------------------------------------
// Run File — runs Main.py <file> --run in a terminal
// ---------------------------------------------------------------------------

function runFile(context) {
    const editor = vscode.window.activeTextEditor;
    if (!editor || !editor.document.fileName.endsWith('.ul')) {
        vscode.window.showWarningMessage("Open a .ul file first.");
        return;
    }

    const config = vscode.workspace.getConfiguration("unilogic");
    let pythonPath = config.get("pythonPath", "");
    if (!pythonPath) pythonPath = findPython();
    if (!pythonPath) { vscode.window.showErrorMessage("UniLogic: Python not found."); return; }

    const mainPy = findMainPy(editor.document.fileName);
    if (!mainPy) {
        vscode.window.showErrorMessage(
            'Main.py not found. Set unilogic.compilerPath in settings to your XPile compiler directory.',
            'Open Settings'
        ).then(c => { if (c === 'Open Settings') vscode.commands.executeCommand('workbench.action.openSettings', 'unilogic.compilerPath'); });
        return;
    }

    editor.document.save();
    const terminal = vscode.window.createTerminal({ name: "UniLogic" });
    terminal.show();
    terminal.sendText(`& "${pythonPath}" "${mainPy}" "${editor.document.fileName}" --run`);
}

// ---------------------------------------------------------------------------
// Compile File — asks target, runs compiler, reports output path
// ---------------------------------------------------------------------------

async function compileFile(context) {
    const editor = vscode.window.activeTextEditor;
    if (!editor || !editor.document.fileName.endsWith('.ul')) {
        vscode.window.showWarningMessage("Open a .ul file first.");
        return;
    }

    const config = vscode.workspace.getConfiguration("unilogic");
    let pythonPath = config.get("pythonPath", "");
    if (!pythonPath) pythonPath = findPython();
    if (!pythonPath) { vscode.window.showErrorMessage("UniLogic: Python not found."); return; }

    const mainPy = findMainPy(editor.document.fileName);
    if (!mainPy) {
        vscode.window.showErrorMessage(
            'Main.py not found. Set unilogic.compilerPath in settings to your XPile compiler directory.',
            'Open Settings'
        ).then(c => { if (c === 'Open Settings') vscode.commands.executeCommand('workbench.action.openSettings', 'unilogic.compilerPath'); });
        return;
    }

    const targets = [
        { label: "C", flag: "c", ext: ".c" },
        { label: "Python", flag: "python", ext: ".py" },
        { label: "JavaScript", flag: "js", ext: ".js" },
        { label: "LLVM IR", flag: "llvm", ext: ".ll" },
        { label: "WebAssembly", flag: "wasm", ext: ".wasm" },
    ];

    const pick = await vscode.window.showQuickPick(targets, {
        placeHolder: "Select compilation target",
        title: "UniLogic: Compile File",
    });
    if (!pick) return;

    editor.document.save();
    const inputPath = editor.document.fileName;
    const outputPath = inputPath.replace(/\.ul$/, pick.ext);

    try {
        const { execFileSync } = require('child_process');
        execFileSync(pythonPath, [mainPy, inputPath, "-t", pick.flag, "-o", outputPath], {
            cwd: path.dirname(mainPy),
            stdio: ['pipe', 'pipe', 'pipe'],
            timeout: 30000,
        });
        vscode.window.showInformationMessage(`Compiled to ${pick.label}: ${outputPath}`, "Open").then(c => {
            if (c === "Open") vscode.window.showTextDocument(vscode.Uri.file(outputPath));
        });
    } catch (err) {
        const stderr = err.stderr ? err.stderr.toString().trim() : err.message;
        vscode.window.showErrorMessage(`Compile error: ${stderr.split('\n').pop()}`);
    }
}

// ---------------------------------------------------------------------------
// Normalization View — slider 0-3 with side-by-side original/normalized
// ---------------------------------------------------------------------------

function showNormView(context) {
    const editor = vscode.window.activeTextEditor;
    if (!editor || !editor.document.fileName.endsWith('.ul')) {
        vscode.window.showWarningMessage("Open a .ul file first.");
        return;
    }

    const config = vscode.workspace.getConfiguration("unilogic");
    let pythonPath = config.get("pythonPath", "");
    if (!pythonPath) pythonPath = findPython();
    if (!pythonPath) { vscode.window.showErrorMessage("UniLogic: Python not found."); return; }

    const ulPath = editor.document.fileName;
    const fileName = path.basename(ulPath);
    const source = editor.document.getText();

    const mainPy = findMainPy(ulPath);
    if (!mainPy) {
        vscode.window.showErrorMessage('Main.py not found. Set unilogic.compilerPath in settings.');
        return;
    }

    const panel = vscode.window.createWebviewPanel(
        "unilogicNorm", `Normalize: ${fileName}`, vscode.ViewColumn.Beside, { enableScripts: true }
    );

    function updatePanel(level) {
        let normalized = source;
        try {
            const { execFileSync } = require('child_process');
            const result = execFileSync(pythonPath, [mainPy, ulPath, "--dry-run", "--norm-level", String(level)], {
                cwd: path.dirname(mainPy), timeout: 10000,
            });
            normalized = result.toString();
        } catch (err) {
            normalized = `// Error at level ${level}: ${err.message}`;
        }

        panel.webview.html = `<!DOCTYPE html>
<html><head><meta charset="UTF-8"><style>
body { font-family: var(--vscode-font-family); color: var(--vscode-foreground); background: var(--vscode-editor-background); margin: 0; padding: 16px; }
.controls { margin-bottom: 12px; }
.controls label { font-weight: 600; }
.controls input[type=range] { width: 200px; vertical-align: middle; }
.level-desc { opacity: 0.6; margin-left: 8px; }
.panels { display: flex; gap: 16px; }
.panel { flex: 1; }
.panel h3 { font-size: 0.9em; opacity: 0.6; margin: 0 0 6px 0; }
pre { background: rgba(255,255,255,0.03); border: 1px solid rgba(255,255,255,0.1); padding: 12px; font-size: 0.85em; overflow-x: auto; white-space: pre; tab-size: 2; line-height: 1.5; }
</style></head><body>
<div class="controls">
  <label>Normalization Level: <span id="val">${level}</span></label>
  <input type="range" min="0" max="3" value="${level}" id="slider">
  <span class="level-desc" id="desc">${["None", "Compound expansion", "Call flattening", "Full normalization"][level]}</span>
</div>
<div class="panels">
  <div class="panel"><h3>Original</h3><pre>${escapeHtml(source)}</pre></div>
  <div class="panel"><h3>Normalized (Level ${level})</h3><pre>${escapeHtml(normalized)}</pre></div>
</div>
<script>
const vscode = acquireVsCodeApi();
const descs = ["None", "Compound expansion", "Call flattening", "Full normalization"];
document.getElementById('slider').addEventListener('input', e => {
    const v = e.target.value;
    document.getElementById('val').textContent = v;
    document.getElementById('desc').textContent = descs[v];
    vscode.postMessage({ command: "setLevel", level: parseInt(v) });
});
</script></body></html>`;
    }

    updatePanel(0);
    panel.webview.onDidReceiveMessage(msg => {
        if (msg.command === "setLevel") updatePanel(msg.level);
    });
}

// ---------------------------------------------------------------------------
// Profile Viewer — reads ul_profile_summary.json, shows tables + flame graph
// ---------------------------------------------------------------------------

async function showProfile(context) {
    const folders = vscode.workspace.workspaceFolders;
    if (!folders) { vscode.window.showErrorMessage("Open a workspace folder first."); return; }

    const profilePath = path.join(folders[0].uri.fsPath, "ul_profile_summary.json");
    if (!fs.existsSync(profilePath)) {
        vscode.window.showInformationMessage("No profile data found. Run your program with --profile flag first.");
        return;
    }

    let profile;
    try {
        profile = JSON.parse(fs.readFileSync(profilePath, 'utf-8'));
    } catch (err) {
        vscode.window.showErrorMessage(`Failed to read profile: ${err.message}`);
        return;
    }

    const panel = vscode.window.createWebviewPanel(
        "unilogicProfile", "UniLogic: Profile", vscode.ViewColumn.One, { enableScripts: true }
    );

    const funcsObj = profile.functions || {};
    const memory = profile.memory || {};
    const dr = profile.dr || {};

    // Convert dict to sorted array, convert ns to ms
    const funcs = Object.entries(funcsObj).map(([name, s]) => ({
        name,
        calls: s.calls || 0,
        total_ms: (s.total_ns || 0) / 1e6,
        avg_ms: (s.avg_ns || 0) / 1e6,
    })).sort((a, b) => b.total_ms - a.total_ms);

    const funcRows = funcs.map(f =>
        `<tr><td>${escapeHtml(f.name)}</td><td>${f.calls}</td><td>${f.total_ms.toFixed(2)}</td><td>${f.avg_ms.toFixed(3)}</td></tr>`
    ).join('');

    panel.webview.html = `<!DOCTYPE html>
<html><head><meta charset="UTF-8"><style>
body { font-family: var(--vscode-font-family); color: var(--vscode-foreground); background: var(--vscode-editor-background); padding: 16px; }
h2 { font-size: 1em; color: var(--vscode-textLink-foreground, #4fc1ff); margin-top: 20px; }
table { border-collapse: collapse; width: 100%; font-size: 0.9em; margin-bottom: 16px; }
th, td { padding: 6px 12px; border: 1px solid rgba(255,255,255,0.1); text-align: left; }
th { background: rgba(255,255,255,0.05); cursor: pointer; }
th:hover { text-decoration: underline; }
.metric { display: inline-block; margin: 4px 16px 4px 0; }
.metric-label { opacity: 0.6; }
.metric-value { font-weight: 600; }
</style></head><body>
<h1>UniLogic Profile</h1>

<h2>Function Call Timers</h2>
${funcs.length ? `<table id="ftable">
<thead><tr><th onclick="sortTable(0)">Function</th><th onclick="sortTable(1)">Calls</th><th onclick="sortTable(2)">Total (ms)</th><th onclick="sortTable(3)">Avg (ms)</th></tr></thead>
<tbody>${funcRows}</tbody></table>` : '<p>No function timing data.</p>'}

<h2>Memory Usage</h2>
<div>
<span class="metric"><span class="metric-label">Total allocations: </span><span class="metric-value">${memory.total_allocs || 0}</span></span>
<span class="metric"><span class="metric-label">Peak live bytes: </span><span class="metric-value">${memory.peak_live_bytes || 0}</span></span>
<span class="metric"><span class="metric-label">Leak candidates: </span><span class="metric-value">${memory.leak_candidates || 0}</span></span>
</div>

<h2>DR Events</h2>
<div>
<span class="metric"><span class="metric-label">GC collections: </span><span class="metric-value">${dr.gc_collects || 0}</span></span>
<span class="metric"><span class="metric-label">Refcount operations: </span><span class="metric-value">${dr.refcount_ops || 0}</span></span>
<span class="metric"><span class="metric-label">Arena resets: </span><span class="metric-value">${dr.arena_resets || 0}</span></span>
</div>

<script>
function sortTable(col) {
    const table = document.getElementById('ftable');
    const rows = Array.from(table.querySelectorAll('tbody tr'));
    const isNum = col > 0;
    rows.sort((a, b) => {
        const av = a.cells[col].textContent;
        const bv = b.cells[col].textContent;
        return isNum ? parseFloat(bv) - parseFloat(av) : av.localeCompare(bv);
    });
    const tbody = table.querySelector('tbody');
    rows.forEach(r => tbody.appendChild(r));
}
</script></body></html>`;
}

// ---------------------------------------------------------------------------
// Run with Profile — runs with --profile flag, then opens profile viewer
// ---------------------------------------------------------------------------

function runWithProfile(context) {
    const editor = vscode.window.activeTextEditor;
    if (!editor || !editor.document.fileName.endsWith('.ul')) {
        vscode.window.showWarningMessage("Open a .ul file first.");
        return;
    }

    const config = vscode.workspace.getConfiguration("unilogic");
    let pythonPath = config.get("pythonPath", "");
    if (!pythonPath) pythonPath = findPython();
    if (!pythonPath) { vscode.window.showErrorMessage("UniLogic: Python not found."); return; }

    const mainPy = findMainPy(editor.document.fileName);
    if (!mainPy) {
        vscode.window.showErrorMessage(
            'Main.py not found. Set unilogic.compilerPath in settings.',
            'Open Settings'
        ).then(c => { if (c === 'Open Settings') vscode.commands.executeCommand('workbench.action.openSettings', 'unilogic.compilerPath'); });
        return;
    }

    editor.document.save();
    const terminal = vscode.window.createTerminal({ name: "UniLogic Profile" });
    terminal.show();
    terminal.sendText(`& "${pythonPath}" "${mainPy}" "${editor.document.fileName}" --run --profile`);

    // After a delay, try to open the profile viewer
    setTimeout(() => { showProfile(context); }, 5000);
}

// ---------------------------------------------------------------------------
// DR Monitor — real-time file watcher on ul_profile.log
// ---------------------------------------------------------------------------

function showDrMonitor(context) {
    const folders = vscode.workspace.workspaceFolders;
    if (!folders) { vscode.window.showErrorMessage("Open a workspace folder first."); return; }

    const logPath = path.join(folders[0].uri.fsPath, "ul_profile.log");

    const panel = vscode.window.createWebviewPanel(
        "unilogicDrMonitor", "UniLogic: DR Monitor", vscode.ViewColumn.Beside, { enableScripts: true }
    );

    let gcCount = 0, refcountDelta = 0, arenaResets = 0, allocs = 0, frees = 0, liveBytes = 0;
    let events = [];

    function updatePanel() {
        const eventsHTML = events.slice(-50).reverse().map(e => {
            const color = { alloc: '#a6e3a1', free: '#f38ba8', gc_collect: '#4a9ad0', refcount: '#fab387', arena_reset: '#cba6f7' }[e.event] || '#a6adc8';
            return `<div class="event" style="border-left: 3px solid ${color}">${escapeHtml(JSON.stringify(e))}</div>`;
        }).join('');

        panel.webview.html = `<!DOCTYPE html>
<html><head><meta charset="UTF-8"><style>
body { font-family: var(--vscode-font-family); color: var(--vscode-foreground); background: var(--vscode-editor-background); padding: 16px; }
h2 { font-size: 1em; color: var(--vscode-textLink-foreground, #4fc1ff); }
.metrics { display: flex; flex-wrap: wrap; gap: 20px; margin-bottom: 16px; }
.metric { background: rgba(255,255,255,0.03); border: 1px solid rgba(255,255,255,0.1); padding: 12px 16px; border-radius: 4px; }
.metric-value { font-size: 1.5em; font-weight: 700; }
.metric-label { opacity: 0.6; font-size: 0.85em; }
.event { padding: 4px 8px; margin: 2px 0; font-size: 0.8em; font-family: monospace; opacity: 0.8; }
.log { max-height: 300px; overflow-y: auto; }
</style></head><body>
<h1>DR Monitor</h1>
<div class="metrics">
  <div class="metric"><div class="metric-value">${gcCount}</div><div class="metric-label">GC Collections</div></div>
  <div class="metric"><div class="metric-value">${refcountDelta}</div><div class="metric-label">Refcount Ops</div></div>
  <div class="metric"><div class="metric-value">${arenaResets}</div><div class="metric-label">Arena Resets</div></div>
  <div class="metric"><div class="metric-value">${allocs}</div><div class="metric-label">Allocations</div></div>
  <div class="metric"><div class="metric-value">${frees}</div><div class="metric-label">Frees</div></div>
  <div class="metric"><div class="metric-value">${liveBytes}</div><div class="metric-label">Live Bytes</div></div>
</div>
<h2>Event Log (latest 50)</h2>
<div class="log">${eventsHTML || '<p style="opacity:0.5">Waiting for events... Run a profiled program to generate ul_profile.log</p>'}</div>
</body></html>`;
    }

    updatePanel();

    // Watch for changes to the log file
    const watcher = vscode.workspace.createFileSystemWatcher(
        new vscode.RelativePattern(folders[0], "ul_profile.log")
    );

    function processLog() {
        if (!fs.existsSync(logPath)) return;
        try {
            const lines = fs.readFileSync(logPath, 'utf-8').trim().split('\n');
            events = [];
            gcCount = 0; refcountDelta = 0; arenaResets = 0; allocs = 0; frees = 0; liveBytes = 0;
            for (const line of lines) {
                if (!line.trim()) continue;
                try {
                    const e = JSON.parse(line);
                    events.push(e);
                    if (e.event === 'gc_collect') gcCount++;
                    else if (e.event === 'refcount') refcountDelta++;
                    else if (e.event === 'arena_reset') arenaResets++;
                    else if (e.event === 'alloc') { allocs++; liveBytes += (e.size || 0); }
                    else if (e.event === 'free') { frees++; liveBytes -= (e.size || 0); }
                } catch {}
            }
            updatePanel();
        } catch {}
    }

    processLog();
    watcher.onDidChange(() => processLog());
    watcher.onDidCreate(() => processLog());

    panel.onDidDispose(() => watcher.dispose());
    context.subscriptions.push(watcher);
}

// ---------------------------------------------------------------------------
// Hot Path Highlighter — decorates hot functions after profiling
// ---------------------------------------------------------------------------

let hotPathDecorType = null;

function applyHotPathHighlights(context) {
    const folders = vscode.workspace.workspaceFolders;
    if (!folders) { vscode.window.showErrorMessage("Open a workspace folder first."); return; }

    const profilePath = path.join(folders[0].uri.fsPath, "ul_profile_summary.json");
    if (!fs.existsSync(profilePath)) {
        vscode.window.showInformationMessage("No profile data found. Run with --profile first.");
        return;
    }

    const profile = JSON.parse(fs.readFileSync(profilePath, 'utf-8'));
    const funcsObj = profile.functions || {};

    // Calculate total time
    let totalNs = 0;
    for (const s of Object.values(funcsObj)) totalNs += (s.total_ns || 0);
    if (totalNs === 0) { vscode.window.showInformationMessage("No timing data in profile."); return; }

    // Find hot functions (>10% of total time)
    const hotFuncs = {};
    for (const [name, s] of Object.entries(funcsObj)) {
        const pct = (s.total_ns / totalNs) * 100;
        if (pct >= 10) {
            hotFuncs[name] = { pct, calls: s.calls, avg_ms: (s.avg_ns || 0) / 1e6 };
        }
    }

    if (Object.keys(hotFuncs).length === 0) {
        vscode.window.showInformationMessage("No hot paths found (no function used >10% of total time).");
        return;
    }

    // Create decoration type
    if (hotPathDecorType) hotPathDecorType.dispose();
    hotPathDecorType = vscode.window.createTextEditorDecorationType({
        backgroundColor: 'rgba(255, 140, 0, 0.15)',
        overviewRulerColor: 'rgba(255, 140, 0, 0.7)',
        overviewRulerLane: vscode.OverviewRulerLane.Center,
    });
    context.subscriptions.push(hotPathDecorType);

    // Apply to visible editors
    for (const editor of vscode.window.visibleTextEditors) {
        if (!editor.document.fileName.endsWith('.ul')) continue;
        const decorations = [];
        const lines = editor.document.getText().split('\n');
        for (let i = 0; i < lines.length; i++) {
            const match = lines[i].match(/^function\s+(\w+)\s*\(/);
            if (match && hotFuncs[match[1]]) {
                const h = hotFuncs[match[1]];
                decorations.push({
                    range: new vscode.Range(i, 0, i, lines[i].length),
                    hoverMessage: `Hot path: ${h.pct.toFixed(1)}% of execution time, called ${h.calls} times, avg ${h.avg_ms.toFixed(3)}ms`,
                });
            }
        }
        editor.setDecorations(hotPathDecorType, decorations);
    }

    vscode.window.showInformationMessage(`Highlighted ${Object.keys(hotFuncs).length} hot path(s).`);
}

// ---------------------------------------------------------------------------
// Toggle Function Logging — stores per-function profiling toggle in workspace state
// ---------------------------------------------------------------------------

function toggleFunctionLogging(context) {
    const editor = vscode.window.activeTextEditor;
    if (!editor || !editor.document.fileName.endsWith('.ul')) {
        vscode.window.showWarningMessage("Open a .ul file first.");
        return;
    }

    // Find the function the cursor is inside
    const cursorLine = editor.selection.active.line;
    const lines = editor.document.getText().split('\n');
    let funcName = null;

    for (let i = cursorLine; i >= 0; i--) {
        const match = lines[i].match(/^function\s+(\w+)\s*\(/);
        if (match) { funcName = match[1]; break; }
    }

    if (!funcName) {
        vscode.window.showWarningMessage("Place cursor inside a function to toggle logging.");
        return;
    }

    // Get current set from workspace state
    const key = 'unilogic.profileFunctions';
    const current = context.workspaceState.get(key, []);
    const idx = current.indexOf(funcName);
    if (idx >= 0) {
        current.splice(idx, 1);
        vscode.window.showInformationMessage(`Profiling disabled for '${funcName}'. Active: ${current.join(', ') || 'none'}`);
    } else {
        current.push(funcName);
        vscode.window.showInformationMessage(`Profiling enabled for '${funcName}'. Active: ${current.join(', ')}`);
    }
    context.workspaceState.update(key, current);
}

function deactivate() {
    if (explainDecorationType) {
        explainDecorationType.dispose();
    }
    if (cOutputWatcher) {
        cOutputWatcher.dispose();
    }
    if (client) {
        return client.stop();
    }
}

module.exports = { activate, deactivate };
