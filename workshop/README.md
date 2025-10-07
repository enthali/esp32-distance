# Workshop: Embedded GitHub Coding (Materialien)

Dieses Verzeichnis enthält die Materialien für den Workshop. Ziel ist es, Teilnehmenden praxisnah zu zeigen, wie man an Open-Source-Embedded-Projekten (hier: ESP32 + HC-SR04 + WS2812) mit GitHub, Codespaces und Copilot/Coding-Agenten zusammenarbeitet.

Enthaltene Dateien (kurze Übersicht):

- `WORKSHOP-BRAINSTORMING.md` — Agenda, Lernziele und Zeitplan
- `COPILOT-PROMPTS.md` — Beispiel-Prompts für Copilot / Coding-Agent
- `CODESPACES-GUIDE.md` — Schritt-für-Schritt: Codespace / Dev Container öffnen
- `BUILD-FLASH-INSTRUCTIONS.md` — Build- und Flash-Anleitung (ESP-IDF)
- `ISSUE-MCP-EXERCISE.md` — Übung: Issue formulieren & Coding-Agent nutzen

## Voraussetzungen

- Git & GitHub-Zugang
- VS Code (empfohlen) mit Dev Container / Codespaces oder lokale ESP-IDF-Umgebung
- USB-Kabel und ein ESP32-Entwicklungsboard zum Flashen (optional für Hands-on)

## Kurzablauf

1. Setup prüfen (Codespace oder lokaler Dev Container)
2. Einführung in Repository-Struktur und Komponenten
3. Kleine Aufgaben/Issues: messen, anzeigen, LED-Pattern erweitern
4. Einsatz von Copilot/Coding-Agenten zur Unterstützung
5. Code-Review, Tests, Merge-Prozess und Wrap-up

## Workshop-Übungen (Beispiele)

- Aufgabe A: LED-Muster erweitern (Score: Lesbarkeit & Speichereffizienz)
- Aufgabe B: Robustere Distanzmessung (Timeouts, Filter)
- Aufgabe C: HTTPS-Webinterface testen (Zertifikate im Build)

## FAQ / Troubleshooting (Kurz)

Q: Codespaces nicht verfügbar?

A: Nutze lokal VS Code mit dem im Repo enthaltenen Dev-Container oder folge `CODESPACES-GUIDE.md`.

Q: Copilot liefert unbrauchbare Vorschläge?

A: Mehr Kontext in Prompt geben (z. B. REQ-IDs, Dateipfade, gewünschte Rückgabewerte). Immer Vorschläge prüfen — nie blind übernehmen.

Q: Der Coding-Agent hat falsche Änderungen gemacht?

A: Agent stoppen, Branch prüfen, Änderungen manuell anpassen. PR-Review-Prozess nutzen (Branch → PR → Review → Merge).

## Contribution & Next Steps

- Bevor du Änderungen pushst: lokal Commit-Nachricht im Format beachten (siehe `/.github/prompt-snippets/commit-message.md`).
- Vorschlag für den ersten lokalen Commit (nach Korrekturen):

```bash
git checkout -b workshop
git add workshop/README.md
git commit -m "docs(workshop): Überarbeite Workshop-README (DE)"
```

- Optional: PR von `workshop` → `main` erstellen, Review anfordern.

---

Diese Dateien wurden als Ausgangspunkt für Workshop-Materialien in das Verzeichnis `workshop/` kopiert und übersetzt bzw. ergänzt.
