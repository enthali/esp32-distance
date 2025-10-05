let mermaidInitialized = false;

function getMermaidTheme() {
  const scheme = document.documentElement.getAttribute("data-md-color-scheme");
  return scheme === "slate" ? "dark" : "default";
}

function renderMermaidDiagrams() {
  if (!window.mermaid) {
    return;
  }

  if (!mermaidInitialized) {
    window.mermaid.initialize({
      startOnLoad: false,
      theme: getMermaidTheme(),
    });
    mermaidInitialized = true;
  }

  window.mermaid.run({
    querySelector: ".mermaid",
  });
}

document$.subscribe(() => {
  renderMermaidDiagrams();
});

const colorSchemeQuery = window.matchMedia ? window.matchMedia("(prefers-color-scheme: dark)") : null;
if (colorSchemeQuery) {
  const listener = () => {
    mermaidInitialized = false;
    renderMermaidDiagrams();
  };

  if (typeof colorSchemeQuery.addEventListener === "function") {
    colorSchemeQuery.addEventListener("change", listener);
  } else if (typeof colorSchemeQuery.addListener === "function") {
    colorSchemeQuery.addListener(listener);
  }
}
