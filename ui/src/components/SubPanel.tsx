import type { ReactNode } from "react";
import Paper from "@mui/material/Paper";
import Typography from "@mui/material/Typography";
import Box from "@mui/material/Box";

export interface SubPanelProps {
  title: string;
  children: ReactNode;
  sx?: Record<string, unknown>;
}

export function SubPanel({ title, children, sx }: SubPanelProps) {
  return (
    <Paper
      elevation={0}
      sx={{
        display: "flex",
        flexDirection: "column",
        bgcolor: "rgba(255,255,255,0.03)",
        borderRadius: 1.5,
        p: 1,
        gap: 1.5,
        ...sx,
      }}
    >
      <Typography
        variant="overline"
        sx={{
          fontSize: 9,
          letterSpacing: 1.2,
          fontStyle: "normal",
          fontWeight: 700,
          color: "text.secondary",
          textAlign: "center",
          lineHeight: 1,
        }}
      >
        {title}
      </Typography>
      <Box sx={{ display: "flex", flex: 1, alignItems: "flex-start", justifyContent: "center", gap: 1 }}>
        {children}
      </Box>
    </Paper>
  );
}
