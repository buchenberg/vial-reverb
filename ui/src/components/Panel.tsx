import type { ReactNode } from "react";
import Paper from "@mui/material/Paper";
import Typography from "@mui/material/Typography";
import Box from "@mui/material/Box";

export interface PanelProps {
  title: string;
  kind: "reverb";
  children: ReactNode;
}

export function Panel({ title, kind, children }: PanelProps) {
  return (
    <Paper
      elevation={0}
      sx={{
        display: "flex",
        flexDirection: "column",
        flex: 1,
        minHeight: 0,
        bgcolor: "background.paper",
        borderRadius: 2,
        overflow: "hidden",
      }}
      data-panel={kind}
    >
      <Box
        sx={{
          display: "flex",
          alignItems: "center",
          justifyContent: "center",
          px: 2,
          py: 1.25,
          bgcolor: "rgba(0,0,0,0.15)",
          borderBottom: "1px solid rgba(255,255,255,0.04)",
        }}
      >
        <Typography variant="overline" sx={{ fontWeight: 600, letterSpacing: 1 }}>
          {title}
        </Typography>
      </Box>
      <Box sx={{ flex: 1, display: "flex", flexDirection: "column", gap: 1.5, p: 1.5, minWidth: 0, minHeight: 0 }}>
        {children}
      </Box>
    </Paper>
  );
}
