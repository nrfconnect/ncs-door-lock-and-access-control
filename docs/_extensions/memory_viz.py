"""
Copyright (c) 2026 Nordic Semiconductor ASA

SPDX-License-Identifier: LicenseRef-Nordic-5-Clause

Sphinx extension for application memory bar charts.
"""

from __future__ import annotations

import html
from pathlib import Path
from typing import Any

from docutils import nodes
from docutils.parsers.rst import directives
from docutils.statemachine import StringList
from memory_data import (
    applicable_samples,
    board_external_total_kb,
    board_nvm_total_kb,
    board_ram_total_kb,
    load_board_data,
    resolve_layout,
)
from sphinx.application import Sphinx
from sphinx.util.docutils import SphinxDirective

__version__ = "0.1.0"

RESOURCES_DIR = Path(__file__).parent / "static"

PARTITION_COLORS = {
    "boot": ("#2563eb", "#93c5fd"),
    "tfm": ("#7c3aed", "#ddd6fe"),
    "slot0": ("#16a34a", "#bbf7d0"),
    "slot1": ("#0369a1", "#7dd3fc"),
    "factory_data": ("#ea580c", "#fed7aa"),
    "storage": ("#ca8a04", "#fde68a"),
    "tfm_storage": ("#0d9488", "#99f6e4"),
    "padding": ("#94a3b8", "#e2e8f0"),
    "ram": ("#0891b2", "#a5f3fc"),
    "unused": ("#e2e8f0", "#f8fafc"),
}

SPLIT_USAGE_IDS = frozenset({"slot0", "slot1", "ram"})
ALWAYS_VISIBLE_BAR_IDS = frozenset(
    {"boot", "tfm", "slot0", "slot1", "factory_data", "storage", "tfm_storage", "padding"}
)
INTERNAL_NVM_LEGEND_ORDER = (
    "boot",
    "tfm",
    "slot0",
    "factory_data",
    "storage",
    "tfm_storage",
    "padding",
)
EXTERNAL_NVM_LEGEND_ORDER = ("slot1", "padding")
TOOLTIP_ALIGN_START_THRESHOLD_PCT = 20.0


def _kb_label(kb: float) -> str:
    if kb == int(kb):
        return f"{int(kb)} kB"
    return f"{kb:.1f} kB"


def _legend_threshold(data: dict[str, Any]) -> float:
    return float(data.get("legend_threshold_kb", 8))


def _partition_colors(part_id: str) -> tuple[str, str]:
    return PARTITION_COLORS.get(part_id, PARTITION_COLORS["unused"])


def _used_free_pcts(used_kb: float, size_kb: float) -> tuple[float, float]:
    used_kb = min(float(used_kb), size_kb)
    used_pct = min(100.0, 100.0 * used_kb / size_kb) if size_kb else 0.0
    return used_pct, 100.0 - used_pct


def _usage_title(part: dict[str, Any], used_kb: float) -> str:
    size_kb = float(part["size_kb"])
    used_kb = min(float(used_kb), size_kb)
    free_kb = max(size_kb - used_kb, 0)
    label = part["label"]
    if free_kb <= 0:
        used_pct, _ = _used_free_pcts(used_kb, size_kb)
        return f"{label}: ({_kb_label(used_kb)} used, {used_pct:.0f}%)"
    used_pct, free_pct = _used_free_pcts(used_kb, size_kb)
    return (
        f"{label}: ({_kb_label(used_kb)} used, {_kb_label(free_kb)} free "
        f"({used_pct:.0f}% used, {free_pct:.0f}% free))"
    )


def _tooltip_align_class(segment_start_pct: float, width_pct: float) -> str:
    center_pct = segment_start_pct + width_pct / 2.0
    if center_pct < TOOLTIP_ALIGN_START_THRESHOLD_PCT:
        return " memory-viz-segment-tooltip-start"
    return ""


def _render_segment(
    part: dict[str, Any],
    total_kb: float,
    used_kb: float | None,
    legend_only: bool,
    threshold_kb: float,
    segment_start_pct: float,
) -> tuple[str, float]:
    if legend_only:
        return "", 0.0

    used_color, free_color = _partition_colors(part["id"])
    size_kb = float(part["size_kb"])
    width_pct = 100.0 * size_kb / total_kb if total_kb else 0.0
    min_width_pct = 100.0 * threshold_kb / total_kb if total_kb else 0.0
    if part["id"] not in ALWAYS_VISIBLE_BAR_IDS and width_pct < min_width_pct:
        return "", 0.0

    title = f'{part["label"]}: {_kb_label(size_kb)}'
    inner = (
        f'<span class="memory-viz-fill memory-viz-fill-used" '
        f'style="width:100%;background:{used_color}"></span>'
    )

    if used_kb is not None and part["id"] in SPLIT_USAGE_IDS:
        title = _usage_title(part, used_kb)
        used_pct, free_pct = _used_free_pcts(used_kb, size_kb)
        inner = (
            f'<span class="memory-viz-fill memory-viz-fill-used" '
            f'style="width:{used_pct:.2f}%;background:{used_color}"></span>'
            f'<span class="memory-viz-fill memory-viz-fill-free" '
            f'style="width:{free_pct:.2f}%;background:{free_color}"></span>'
        )

    tooltip_class = _tooltip_align_class(segment_start_pct, width_pct)
    segment = (
        f'<div class="memory-viz-segment memory-viz-segment-{part["id"]}{tooltip_class}" '
        f'style="flex:0 0 {width_pct:.4f}%" '
        f'data-tooltip="{html.escape(title, quote=True)}">'
        f"{inner}</div>"
    )
    return segment, width_pct


def _render_bar(
    partitions: list[dict[str, Any]],
    total_kb: float,
    usage: dict[str, float],
    threshold_kb: float,
    aria_label: str,
) -> str:
    segments: list[str] = []
    segment_start_pct = 0.0
    for part in partitions:
        legend_only = part.get("legend_only", False)
        if part["id"] in ALWAYS_VISIBLE_BAR_IDS:
            legend_only = False
        used_kb = usage.get(part["id"])
        segment, width_pct = _render_segment(
            part,
            total_kb,
            used_kb,
            legend_only,
            threshold_kb,
            segment_start_pct,
        )
        if segment:
            segments.append(segment)
            segment_start_pct += width_pct

    return (
        f'<div class="memory-viz-bar" role="img" aria-label="{html.escape(aria_label)}">'
        f'{"".join(segments)}</div>'
    )


def _legend_swatch(color: str) -> str:
    return f'<span class="memory-viz-swatch" style="background:{color}"></span>'


def _legend_item(part_id: str, label: str) -> str:
    return (
        f'<span class="memory-viz-legend-item">'
        f"{_legend_swatch(_partition_colors(part_id)[0])}"
        f"{html.escape(label)}"
        f"</span>"
    )


def _legend_items_used_free(part_id: str, label_base: str) -> list[str]:
    used_color, free_color = _partition_colors(part_id)
    items: list[str] = []
    for suffix, color in (("used", used_color), ("free", free_color)):
        items.append(
            f'<span class="memory-viz-legend-item">'
            f"{_legend_swatch(color)}"
            f"{html.escape(label_base)} ({suffix})"
            f"</span>"
        )
    return items


def _collect_legend_items(
    parts: dict[str, dict[str, Any]],
    order: tuple[str, ...],
) -> list[str]:
    order_index = {part_id: index for index, part_id in enumerate(order)}
    sorted_ids = sorted(
        parts,
        key=lambda part_id: (order_index.get(part_id, len(order)), part_id),
    )

    items: list[str] = []
    for part_id in sorted_ids:
        part = parts[part_id]
        label = part["label"]
        if part_id in SPLIT_USAGE_IDS:
            items.extend(_legend_items_used_free(part_id, label))
        else:
            items.append(_legend_item(part_id, label))
    return items


def _collect_internal_nvm_legend_items(
    data: dict[str, Any],
    layout_names: set[str],
) -> list[str]:
    parts: dict[str, dict[str, Any]] = {}
    for layout_name in layout_names:
        layout = resolve_layout(data, layout_name)
        for part in layout["partitions"]:
            if part["id"] != "ram" and part.get("memory") != "ram":
                parts.setdefault(part["id"], part)
    return _collect_legend_items(parts, INTERNAL_NVM_LEGEND_ORDER)


def _collect_external_nvm_legend_items(
    data: dict[str, Any],
    layout_names: set[str],
) -> list[str]:
    parts: dict[str, dict[str, Any]] = {}
    for layout_name in layout_names:
        layout = resolve_layout(data, layout_name)
        external = layout.get("external")
        if external:
            for part in external["partitions"]:
                parts.setdefault(part["id"], part)
    return _collect_legend_items(parts, EXTERNAL_NVM_LEGEND_ORDER)


def _render_ram_legend_items() -> list[str]:
    return _legend_items_used_free("ram", "RAM")


def _render_legend_div(css_class: str, items: list[str]) -> str:
    if not items:
        return ""
    return f'<div class="memory-viz-legend {css_class}">{"".join(items)}</div>'


def _render_row_label(text: str) -> str:
    return f'<div class="memory-viz-row-label">{html.escape(text)}</div>'


def _render_bar_stack(
    row_label: str,
    bar_html: str,
    *,
    stack_class: str = "",
    bar_wrap_class: str = "memory-viz-bar-wrap",
    width_pct: float | None = None,
) -> str:
    width_style = f' style="width:{width_pct:.4f}%"' if width_pct is not None else ""
    stack_attrs = f' class="memory-viz-bar-stack{stack_class}"{width_style}'
    return (
        f"<div{stack_attrs}>"
        f"{_render_row_label(row_label)}"
        f'<div class="{bar_wrap_class}">{bar_html}</div>'
        f"</div>"
    )


def _grid_style(nvm_total_kb: float, ram_total_kb: float) -> str:
    return (
        f"--memory-viz-nvm-fr:{int(nvm_total_kb)}fr;"
        f"--memory-viz-ram-fr:{int(ram_total_kb)}fr"
    )


def _parse_rst_label(directive: SphinxDirective, text: str) -> nodes.Element:
    label = nodes.paragraph()
    label["classes"] = ["memory-viz-sample-label"]
    content = StringList([text], "<memory_viz>")
    directive.state.nested_parse(content, 0, label)
    if len(label) == 1 and isinstance(label[0], nodes.paragraph):
        inner = label[0]
        label.remove(inner)
        label.extend(inner.children)
    return label


def _nvm_width_pct(part_kb: float, reference_kb: float) -> float:
    if reference_kb <= 0:
        return 100.0
    return min(100.0, 100.0 * part_kb / reference_kb)


def _render_board_header(
    nvm_total_kb: float,
    ram_total_kb: float,
    external_total_kb: float | None,
    internal_legend_items: list[str],
    external_legend_items: list[str],
    ram_legend_items: list[str],
) -> str:
    internal_legend = _render_legend_div("memory-viz-legend-nvm", internal_legend_items)

    external_title = ""
    external_legend = ""
    if external_total_kb is not None:
        ext_width = _nvm_width_pct(external_total_kb, nvm_total_kb)
        external_title = (
            f'<div class="memory-viz-panel-title memory-viz-panel-title-secondary" '
            f'style="width:{ext_width:.4f}%">'
            f"External NVM ({_kb_label(external_total_kb)})</div>"
        )
        external_legend = _render_legend_div(
            "memory-viz-legend-external",
            external_legend_items,
        )

    ram_legend = _render_legend_div("memory-viz-legend-ram", ram_legend_items)

    return (
        f'<div class="memory-viz-chart-header">'
        f'<div class="memory-viz-panel-title memory-viz-panel-title-sample">Application</div>'
        f'<div class="memory-viz-header-nvm">'
        f'<div class="memory-viz-panel-title">Internal NVM ({_kb_label(nvm_total_kb)})</div>'
        f"{internal_legend}"
        f"{external_title}"
        f"{external_legend}"
        f"</div>"
        f'<div class="memory-viz-header-ram">'
        f'<div class="memory-viz-panel-title">RAM ({_kb_label(ram_total_kb)})</div>'
        f"{ram_legend}"
        f"</div>"
        f"</div>"
    )


def _render_sample_nvm_html(
    sample: dict[str, Any],
    data: dict[str, Any],
    nvm_total_kb: float,
    threshold_kb: float,
) -> str:
    layout = resolve_layout(data, sample["layout"])
    partitions = layout["partitions"]
    layout_total_kb = board_nvm_total_kb(data)
    usage = sample.get("usage", {})
    nvm_partitions = [p for p in partitions if p.get("memory") != "ram"]
    aria = sample.get("label", "sample")

    internal_bar = _render_bar(
        nvm_partitions,
        layout_total_kb,
        usage.get("nvm", {}),
        threshold_kb,
        f"{aria} internal NVM",
    )

    internal_stack = _render_bar_stack("Internal NVM", internal_bar)

    external = layout.get("external")
    external_stack = ""
    if external:
        ext_total_kb = float(external["nvm_total_kb"])
        external_bar = _render_bar(
            external["partitions"],
            ext_total_kb,
            usage.get("external", {}),
            threshold_kb,
            f"{aria} external NVM",
        )
        external_stack = _render_bar_stack(
            "External NVM",
            external_bar,
            stack_class=" memory-viz-bar-stack-external",
            bar_wrap_class="memory-viz-bar-wrap memory-viz-bar-wrap-external",
            width_pct=_nvm_width_pct(ext_total_kb, layout_total_kb),
        )

    return (
        f'<div class="memory-viz-nvm-column">'
        f"{internal_stack}"
        f"{external_stack}"
        f"</div>"
    )


def _render_sample_ram_html(
    sample: dict[str, Any],
    ram_total_kb: float,
    threshold_kb: float,
) -> str:
    usage = sample.get("usage", {})
    ram_used_kb = min(float(usage.get("ram_used_kb", 0)), ram_total_kb)
    ram_partitions = [
        {
            "id": "ram",
            "label": "RAM",
            "size_kb": ram_total_kb,
            "order": 0,
        }
    ]
    aria = sample.get("label", "sample")
    ram_bar = _render_bar(
        ram_partitions,
        ram_total_kb,
        {"ram": ram_used_kb},
        threshold_kb,
        f"{aria} RAM",
    )
    return (
        f'<div class="memory-viz-ram-column">'
        f'{_render_bar_stack("RAM", ram_bar)}'
        f"</div>"
    )


def _render_ram_board_header(ram_total_kb: float, ram_legend_items: list[str]) -> str:
    ram_legend = _render_legend_div("memory-viz-legend-ram", ram_legend_items)
    return (
        f'<div class="memory-viz-chart-header">'
        f'<div class="memory-viz-panel-title memory-viz-panel-title-sample">Application</div>'
        f'<div class="memory-viz-header-ram">'
        f'<div class="memory-viz-panel-title">RAM ({_kb_label(ram_total_kb)})</div>'
        f"{ram_legend}"
        f"</div>"
        f"</div>"
    )


def _build_ram_board_nodes(directive: SphinxDirective, board: str) -> nodes.Element:
    data = load_board_data(board)
    ram_total_kb = float(data["board"]["ram_total_kb"])
    threshold_kb = _legend_threshold(data)
    samples = applicable_samples(data)

    wrapper = nodes.container()
    wrapper["classes"] = ["memory-viz-board-outer"]
    wrapper += nodes.raw(
        "",
        f'<div class="memory-viz-board memory-viz-board-ram-only" data-board="{board}">',
        format="html",
    )

    board_node = nodes.container()
    board_node["classes"] = ["memory-viz-board-inner"]

    board_node += nodes.raw(
        "",
        _render_ram_board_header(ram_total_kb, _render_ram_legend_items()),
        format="html",
    )

    samples_node = nodes.container()
    samples_node["classes"] = ["memory-viz-samples"]

    for sample in samples:
        row = nodes.container()
        row["classes"] = ["memory-viz-sample-row"]
        row += _parse_rst_label(directive, sample["label"])
        row += nodes.raw(
            "",
            _render_sample_ram_html(sample, ram_total_kb, threshold_kb),
            format="html",
        )
        samples_node += row

    board_node += samples_node
    wrapper += board_node
    wrapper += nodes.raw("", "</div>", format="html")
    return wrapper


def _build_board_nodes(directive: SphinxDirective, board: str) -> nodes.Element:
    data = load_board_data(board)
    nvm_total_kb = board_nvm_total_kb(data)
    ram_total_kb = board_ram_total_kb(data) or 0.0
    threshold_kb = _legend_threshold(data)

    samples = applicable_samples(data)
    layout_names = {sample["layout"] for sample in samples if "layout" in sample}
    external_total_kb = board_external_total_kb(data)

    style = _grid_style(nvm_total_kb, ram_total_kb)

    wrapper = nodes.container()
    wrapper["classes"] = ["memory-viz-board-outer"]
    wrapper += nodes.raw(
        "",
        f'<div class="memory-viz-board" data-board="{board}" style="{style}">',
        format="html",
    )

    board_node = nodes.container()
    board_node["classes"] = ["memory-viz-board-inner"]

    internal_legend_items = _collect_internal_nvm_legend_items(data, layout_names)
    external_legend_items = (
        _collect_external_nvm_legend_items(data, layout_names)
        if external_total_kb is not None
        else []
    )
    ram_legend_items = _render_ram_legend_items()

    board_node += nodes.raw(
        "",
        _render_board_header(
            nvm_total_kb,
            ram_total_kb,
            external_total_kb,
            internal_legend_items,
            external_legend_items,
            ram_legend_items,
        ),
        format="html",
    )

    samples_node = nodes.container()
    samples_node["classes"] = ["memory-viz-samples"]

    for sample in samples:
        row = nodes.container()
        row["classes"] = ["memory-viz-sample-row"]
        row += _parse_rst_label(directive, sample["label"])
        row += nodes.raw(
            "",
            _render_sample_nvm_html(sample, data, nvm_total_kb, threshold_kb),
            format="html",
        )
        row += nodes.raw(
            "",
            _render_sample_ram_html(sample, ram_total_kb, threshold_kb),
            format="html",
        )

        samples_node += row

    board_node += samples_node
    wrapper += board_node
    wrapper += nodes.raw("", "</div>", format="html")
    return wrapper


class MemoryBoard(SphinxDirective):
    """Render memory layout charts for a board defined in docs/data/memory/."""

    required_arguments = 0
    optional_arguments = 0
    final_argument_whitespace = True
    has_content = False
    option_spec = {
        "board": directives.unchanged_required,
    }

    def run(self) -> list[nodes.Node]:
        return [_build_board_nodes(self, self.options["board"])]


class RamBoard(SphinxDirective):
    """Render static RAM usage bar charts for a board defined in docs/data/memory/."""

    required_arguments = 0
    optional_arguments = 0
    final_argument_whitespace = True
    has_content = False
    option_spec = {
        "board": directives.unchanged_required,
    }

    def run(self) -> list[nodes.Node]:
        return [_build_ram_board_nodes(self, self.options["board"])]


def add_memory_viz_resources(app: Sphinx) -> None:
    static_path = RESOURCES_DIR.as_posix()
    if static_path not in app.config.html_static_path:
        app.config.html_static_path.append(static_path)
    app.add_css_file("memory_viz.css")


def setup(app: Sphinx) -> dict[str, Any]:
    app.add_directive("memory-board", MemoryBoard)
    app.add_directive("ram-board", RamBoard)
    app.connect("builder-inited", add_memory_viz_resources)
    return {
        "version": __version__,
        "parallel_read_safe": True,
        "parallel_write_safe": True,
    }
