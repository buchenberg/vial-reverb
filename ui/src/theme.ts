import { createTheme } from "@mui/material/styles";

export const theme = createTheme({
  palette: {
    mode: "dark",
    primary: { main: "#6f86e8" },
    background: { default: "#16161c", paper: "#24242c" },
  },
  typography: {
    fontFamily: '"Inter", "Segoe UI", system-ui, sans-serif',
    fontSize: 12,
  },
  components: {
    MuiCssBaseline: {
      styleOverrides: {
        body: { userSelect: "none", WebkitUserSelect: "none", overflow: "hidden" },
      },
    },
    MuiSlider: {
      styleOverrides: {
        root: { height: 4, padding: "8px 0" },
        rail: { opacity: 0.4 },
        thumb: { width: 14, height: 14 },
      },
    },
    MuiTooltip: {
      styleOverrides: {
        tooltip: { fontSize: 11, maxWidth: 220 },
      },
    },
  },
});
