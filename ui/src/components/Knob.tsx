import { useState, useCallback } from "react";
import Box from "@mui/material/Box";
import { useSlider } from "../hooks";
import { PARAM_BY_ID, formatValue } from "../params";
import { Knob as KnobCore } from "react-audio-ui";

export interface KnobProps {
  id: string;
  label: string;
}

export function Knob({ id, label }: KnobProps) {
  const slider = useSlider(id);
  const spec = PARAM_BY_ID[id];
  const [hover, setHover] = useState(false);

  const value = slider.scaled;

  // Double-click to reset to default.
  const onDoubleClick = useCallback(() => {
    if (!spec) return;
    const range = spec.max - spec.min;
    if (range === 0) return;
    const defNorm = (spec.def - spec.min) / range;
    slider.setNormalised(Math.max(0, Math.min(1, defNorm)));
  }, [spec, slider]);

  return (
    <Box
      sx={{ display: "flex", flexDirection: "column", alignItems: "center", width: 72, gap: 0.25 }}
      data-param={id}
      onMouseEnter={() => setHover(true)}
      onMouseLeave={() => setHover(false)}
      onDoubleClick={onDoubleClick}
    >
      <KnobCore
        value={slider.normalised}
        onChange={(v: number) => slider.setNormalised(v)}
        onDragStart={() => slider.dragStart()}
        onDragEnd={() => slider.dragEnd()}
        label={label}
        info={spec?.description}
        valueDisplay={hover ? formatValue(id, value) : undefined}
      />
    </Box>
  );
}
