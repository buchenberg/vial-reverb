import { useEffect, useRef } from "react";
import Box from "@mui/material/Box";
import { useSlider } from "../hooks";

type DrawFn = (ctx: CanvasRenderingContext2D, w: number, h: number, accent: string) => void;

function useCanvas(draw: DrawFn, deps: unknown[]) {
  const ref = useRef<HTMLCanvasElement>(null);
  useEffect(() => {
    const canvas = ref.current;
    if (!canvas) return;
    const rect = canvas.getBoundingClientRect();
    const dpr = window.devicePixelRatio || 1;
    const w = Math.max(1, Math.floor(rect.width));
    const h = Math.max(1, Math.floor(rect.height));
    canvas.width = w * dpr;
    canvas.height = h * dpr;
    let ctx: CanvasRenderingContext2D | null = null;
    try {
      ctx = canvas.getContext("2d");
    } catch {
      return;
    }
    if (!ctx) return;
    ctx.scale(dpr, dpr);
    ctx.clearRect(0, 0, w, h);
    const accent = getComputedStyle(canvas).getPropertyValue("--accent").trim() || "#6f86e8";
    draw(ctx, w, h, accent);
    // eslint-disable-next-line react-hooks/exhaustive-deps
  }, deps);
  return ref;
}

export function ReverbEq({ lowCutId, lowGainId, highCutId, highGainId }: {
  lowCutId: string; lowGainId: string; highCutId: string; highGainId: string;
}) {
  const lc = useSlider(lowCutId).scaled / 128;
  const lg = useSlider(lowGainId).scaled;
  const hc = useSlider(highCutId).scaled / 128;
  const hg = useSlider(highGainId).scaled;
  const ref = useCanvas((ctx, w, h, col) => {
    const mid = h * 0.5;
    const gainToY = (db: number) => mid - (db / 6) * (h * 0.35);
    ctx.beginPath();
    for (let px = 0; px <= w; px++) {
      const x = px / w;
      let db = 0;
      if (x < lc) db += lg * (1 - x / Math.max(0.001, lc));
      if (x > hc) db += hg * ((x - hc) / Math.max(0.001, 1 - hc));
      const y = gainToY(db);
      if (px === 0) ctx.moveTo(px, y); else ctx.lineTo(px, y);
    }
    ctx.strokeStyle = col;
    ctx.lineWidth = 2;
    ctx.stroke();
    ctx.fillStyle = col;
    ctx.beginPath(); ctx.arc(lc * w, gainToY(lg), 5, 0, Math.PI * 2); ctx.fill();
    ctx.beginPath(); ctx.arc(hc * w, gainToY(hg), 5, 0, Math.PI * 2); ctx.fill();
  }, [lc, lg, hc, hg]);
  return (
    <Box
      component="canvas"
      ref={ref}
      sx={{
        width: "100%",
        height: "100%",
        bgcolor: "#1b1b21",
        borderRadius: 1,
        "--accent": "#6f86e8",
      }}
      data-viz="reverb"
    />
  );
}
