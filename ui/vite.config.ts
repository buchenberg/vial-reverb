import { defineConfig } from "vite";
import { resolve } from "path";
import react from "@vitejs/plugin-react";
import { viteSingleFile } from "vite-plugin-singlefile";

// Build a single self-contained index.html (JS + CSS inlined) so the JUCE
// plugin only needs to embed/serve one file via its resource provider.
export default defineConfig(({ command }) => ({
  plugins: [react(), viteSingleFile()],
  resolve: {
    alias: {
      // Resolve the linked library to its source so dev (HMR) and build always
      // use live source instead of a stale pre-bundled dist.
      "react-audio-ui": resolve(__dirname, "../../react-audio-ui/src/lib/index.ts"),
    },
    // Force a single copy of these packages even when the linked library's
    // source imports them from its own node_modules. Without this, a second
    // MUI/emotion instance runs and sx/theme context breaks (e.g. icons
    // render with light-theme defaults inside a dark theme).
    dedupe: [
      "react",
      "react-dom",
      "react/jsx-runtime",
      "@mui/material",
      "@mui/material/styles",
      "@mui/icons-material",
      "@mui/system",
      "@mui/utils",
      "@emotion/react",
      "@emotion/styled",
      "@emotion/cache",
    ],
  },
  optimizeDeps: {
    exclude: command === "serve" ? ["react-audio-ui"] : [],
  },
  build: {
    target: "es2020",
    cssCodeSplit: false,
    assetsInlineLimit: 100000000,
    chunkSizeWarningLimit: 100000,
  },
}));
