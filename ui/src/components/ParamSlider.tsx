import { useState } from "react";
import Box from "@mui/material/Box";
import Slider from "@mui/material/Slider";
import Typography from "@mui/material/Typography";
import Tooltip from "@mui/material/Tooltip";
import InfoOutlined from "@mui/icons-material/InfoOutlined";
import { useSlider } from "../hooks";
import { PARAM_BY_ID, formatValue } from "../params";

export interface ParamSliderProps {
  id: string;
  label: string;
}

export function ParamSlider({ id, label }: ParamSliderProps) {
  const slider = useSlider(id);
  const spec = PARAM_BY_ID[id];
  const [hover, setHover] = useState(false);
  const [dragging, setDragging] = useState(false);

  const norm = slider.normalised;
  const value = slider.scaled;

  const handleChange = (_: Event, newValue: number | number[]) => {
    const v = Array.isArray(newValue) ? newValue[0] : newValue;
    slider.setNormalised(v / 100);
  };

  const showValue = hover || dragging;

  return (
    <Box
      sx={{ display: "flex", flexDirection: "column", alignItems: "center", width: 72, gap: 0.25 }}
      data-param={id}
      onMouseEnter={() => setHover(true)}
      onMouseLeave={() => setHover(false)}
    >
      <Box sx={{ display: "flex", alignItems: "center", justifyContent: "center", gap: 0.25 }}>
        <Typography
          variant="caption"
          sx={{ fontSize: 9, letterSpacing: 0.6, textTransform: "uppercase", color: "text.secondary" }}
        >
          {label}
        </Typography>
        {spec?.description && (
          <Tooltip title={spec.description} arrow placement="top" enterDelay={200}>
            <InfoOutlined
              sx={{ fontSize: 11, color: "text.secondary", opacity: 0.5, cursor: "help", "&:hover": { opacity: 1, color: "primary.main" } }}
            />
          </Tooltip>
        )}
      </Box>
      <Box sx={{ width: "100%", px: 0.5 }}>
        <Slider
          value={norm * 100}
          onChange={handleChange}
          onChangeCommitted={() => setDragging(false)}
          onMouseDown={() => setDragging(true)}
          size="small"
          sx={{ color: "primary.main" }}
        />
      </Box>
      <Typography
        variant="caption"
        sx={{ fontSize: 9, height: 11, color: "primary.main" }}
      >
        {showValue ? formatValue(id, value) : ""}
      </Typography>
    </Box>
  );
}
