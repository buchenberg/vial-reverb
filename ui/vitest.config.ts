import { defineConfig } from "vitest/config";
import react from "@vitejs/plugin-react";
import path from "path";

export default defineConfig({
  plugins: [react()],
  define: { "process.env.NODE_ENV": '"development"' },
  resolve: {
    alias: {
      "react-transition-group/TransitionGroupContext": path.resolve(
        __dirname,
        "node_modules/react-transition-group/cjs/TransitionGroupContext.js",
      ),
    },
  },
  test: {
    globals: true,
    environment: "jsdom",
    setupFiles: ["./src/test/setup.ts"],
    include: ["src/**/*.test.{ts,tsx}"],
  },
});
