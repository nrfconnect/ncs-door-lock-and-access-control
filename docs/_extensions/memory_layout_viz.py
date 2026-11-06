"""
Copyright (c) 2026 Nordic Semiconductor ASA

SPDX-License-Identifier: LicenseRef-Nordic-5-Clause

Sphinx extension for reference partition layout charts.
"""

from __future__ import annotations

import hashlib
import html
from pathlib import Path
from typing import Any

from docutils import nodes
from docutils.parsers.rst import directives
from docutils.statemachine import StringList
from memory_data import load_board_data
from sphinx.application import Sphinx
from sphinx.util.docutils import SphinxDirective

__version__ = "0.2.0"

RESOURCES_DIR = Path(__file__).parent / "static"

_LAYOUT_COLORS = [
    "#2563eb",
    "#16a34a",
    "#7c3aed",
    "#ea580c",
    "#ca8a04",
    "#0d9488",
    "#db2777",
    "#475569",
    "#0891b2",
    "#65a30d",
]

_LAYOUT_FIXED_COLORS = {
    "padding": "#e2e8f0",
    "boot_partition": "#2563eb",
    "slot0_partition": "#16a34a",
    "slot1_partition": "#0369a1",
    "factory_data_partition": "#ea580c",
    "storage_partition": "#ca8a04",
    "tfm_storage_partition": "#0d9488",
}

MAP_MIN_SEGMENT_PX = 28
MAP_TOTAL_HEIGHT_PX = 360


def _color_for_id(part_id: str) -> str:
    if part_id in _LAYOUT_FIXED_COLORS:
        return _LAYOUT_FIXED_COLORS[part_id]
    digest = hashlib.md5(part_id.encode()).hexdigest()
    return _LAYOUT_COLORS[int(digest[:8], 16) % len(_LAYOUT_COLORS)]


def _bytes_label(size_bytes: int) -> str:
    kb = size_bytes / 1024
    if abs(kb - round(kb)) < 0.05:
        return f"{int(round(kb))} kB"
    return f"{kb:.2f} kB"


def _hex_label(value: int) -> str:
    return f"0x{value:x}"


def _absolute_address(base_address: int, offset_bytes: int) -> int:
    return int(base_address) + int(offset_bytes)


def _part_hover_tooltip(part: dict[str, Any], base_address: int) -> str:
    offset_bytes = int(part["offset_bytes"])
    size_bytes = int(part["size_bytes"])
    abs_addr = _absolute_address(base_address, offset_bytes)
    return (
        f'{part["label"]}: offset {_hex_label(abs_addr)}, '
        f"size {_hex_label(size_bytes)} ({_bytes_label(size_bytes)})"
    )


def _render_boundary_rail(abs_address: int) -> str:
    label = html.escape(_hex_label(abs_address))
    return (
        f'<div class="memory-layout-rail">'
        f'<span class="memory-layout-boundary-line" aria-hidden="true"></span>'
        f'<span class="memory-layout-offset">{label}</span>'
        f"</div>"
    )


def _render_empty_rail() -> str:
    return '<div class="memory-layout-rail memory-layout-rail-empty" aria-hidden="true"></div>'


def _flex_weights(sizes_bytes: list[int]) -> list[int]:
    """Compress size ratios so small partitions stay readable (sqrt scaling)."""
    if not sizes_bytes:
        return []
    roots = [max(int(size), 1) ** 0.5 for size in sizes_bytes]
    total = sum(roots) or 1.0
    return [max(1, int(round(100.0 * root / total))) for root in roots]


def _segment_height_px(flex_weight: int, flex_total: int, available_height_px: int) -> int:
    if flex_total <= 0:
        return MAP_MIN_SEGMENT_PX
    height_px = int(round(available_height_px * flex_weight / flex_total))
    return max(MAP_MIN_SEGMENT_PX, height_px)


def _render_leaf_fill(color: str, tooltip: str) -> str:
    return (
        f'<span class="memory-layout-fill memory-layout-vsegment-hover" '
        f'style="background:{color}" data-tooltip="{tooltip}" tabindex="0"></span>'
    )


def _render_vertical_segment(
    part: dict[str, Any],
    flex_weight: int,
    flex_total: int,
    base_address: int,
    *,
    depth: int = 0,
    available_height_px: int = MAP_TOTAL_HEIGHT_PX,
    show_start_marker: bool = True,
    marker_suppress_offset: int | None = None,
) -> str:
    color = _color_for_id(part["id"])
    part_offset = int(part["offset_bytes"])
    abs_addr = _absolute_address(base_address, part_offset)
    tooltip = html.escape(_part_hover_tooltip(part, base_address), quote=True)
    children = part.get("children") or []

    height_px = _segment_height_px(flex_weight, flex_total, available_height_px)

    if children:
        child_sizes = [int(child["size_bytes"]) for child in children]
        child_weights = _flex_weights(child_sizes)
        child_total = sum(child_weights) or 1
        child_html_parts = []
        for child, weight in zip(children, child_weights, strict=True):
            child_html_parts.append(
                _render_vertical_segment(
                    child,
                    weight,
                    child_total,
                    base_address,
                    depth=depth + 1,
                    available_height_px=height_px,
                )
            )
        inner = f'<div class="memory-layout-vstack">{"".join(child_html_parts)}</div>'
        block_class = "memory-layout-block memory-layout-block-nested"
    else:
        inner = _render_leaf_fill(color, tooltip)
        block_class = "memory-layout-block"

    if depth > 0:
        return (
            f'<div class="memory-layout-partition memory-layout-depth-{depth}" '
            f'style="height:{height_px}px;flex:0 0 auto">'
            f'<div class="memory-layout-partition-grid memory-layout-partition-grid-nested">'
            f'<div class="{block_class}">'
            f"{inner}"
            f"</div>"
            f"</div>"
            f"</div>"
        )

    show_marker = show_start_marker and (
        marker_suppress_offset is None or part_offset != marker_suppress_offset
    )
    rail_html = _render_boundary_rail(abs_addr) if show_marker else _render_empty_rail()

    if children:
        block_open = (
            f'<div class="{block_class} memory-layout-vsegment-hover" '
            f'data-tooltip="{tooltip}" tabindex="0">'
        )
    else:
        block_open = f'<div class="{block_class}">'

    return (
        f'<div class="memory-layout-partition memory-layout-depth-{depth}" '
        f'style="height:{height_px}px;flex:0 0 auto">'
        f'<div class="memory-layout-partition-grid">'
        f"{block_open}"
        f"{inner}"
        f"</div>"
        f"{rail_html}"
        f"</div>"
        f"</div>"
    )


def _render_region_legend(
    parts: list[dict[str, Any]],
    collected: list[str] | None = None,
    seen_ids: set[str] | None = None,
) -> list[str]:
    if collected is None:
        collected = []
    if seen_ids is None:
        seen_ids = set()

    for part in parts:
        part_id = part["id"]
        if part_id in seen_ids:
            if part.get("children"):
                _render_region_legend(part["children"], collected, seen_ids)
            continue
        seen_ids.add(part_id)
        swatch = (
            f'<span class="memory-viz-swatch" style="background:{_color_for_id(part_id)}"></span>'
        )
        size_bytes = int(part["size_bytes"])
        label = f'{html.escape(part["label"])} ({_bytes_label(size_bytes)})'
        collected.append(f'<span class="memory-viz-legend-item">{swatch}{label}</span>')
        if part.get("children"):
            _render_region_legend(part["children"], collected, seen_ids)
    return collected


def _render_region_html(region: dict[str, Any]) -> str:
    partitions = region.get("partitions", [])
    total_bytes = int(region["total_bytes"])
    base_address = int(region.get("base_address") or 0)
    legend_items = _render_region_legend(partitions)
    legend = (
        f'<div class="memory-viz-legend memory-layout-legend">{"".join(legend_items)}</div>'
        if legend_items
        else ""
    )

    sizes = [int(part["size_bytes"]) for part in partitions]
    weights = _flex_weights(sizes)
    weight_total = sum(weights) or 1
    segments = []
    for part, weight in zip(partitions, weights, strict=True):
        segments.append(
            _render_vertical_segment(
                part,
                weight,
                weight_total,
                base_address,
                show_start_marker=True,
            )
        )

    segments_html = "".join(segments)
    end_address = base_address + total_bytes
    end_row = (
        f'<div class="memory-layout-partition memory-layout-partition-end">'
        f'<div class="memory-layout-partition-grid">'
        f'<div class="memory-layout-block memory-layout-block-spacer" aria-hidden="true"></div>'
        f"{_render_boundary_rail(end_address)}"
        f"</div>"
        f"</div>"
    )

    note = ""
    if region.get("address_note_rst"):
        note = f'<p class="memory-layout-note">{html.escape(region["address_note_rst"])}</p>'

    return (
        f'<div class="memory-layout-region" data-region="{html.escape(region["id"], quote=True)}">'
        f'<div class="memory-layout-region-title">'
        f'{html.escape(region["title"])} '
        f"(size: {_hex_label(total_bytes)} = {_bytes_label(total_bytes)})"
        f"</div>"
        f"{note}"
        f"{legend}"
        f'<div class="memory-layout-map">'
        f'<div class="memory-layout-map-stack">{segments_html}{end_row}</div>'
        f"</div>"
        f"</div>"
    )


def _format_offset(offset_bytes: int) -> str:
    return f"{offset_bytes} (0x{offset_bytes:x})"


def _format_size(size_bytes: int) -> str:
    return f"{_bytes_label(size_bytes)} ({_hex_label(size_bytes)})"


def _iter_partition_rows(
    partitions: list[dict[str, Any]],
    *,
    depth: int = 0,
) -> list[tuple[str, int, int]]:
    rows: list[tuple[str, int, int]] = []
    indent = "  " * depth
    for part in partitions:
        rows.append((f"{indent}{part['label']}", int(part["offset_bytes"]), int(part["size_bytes"])))
        children = part.get("children") or []
        if children:
            rows.extend(_iter_partition_rows(children, depth=depth + 1))
    return rows


def _append_rst_paragraph(state, parent: nodes.Element, rst_text: str) -> None:
    paragraph = nodes.paragraph()
    content = StringList([rst_text], "<memory_layout_table>")
    state.nested_parse(content, 0, paragraph)
    if len(paragraph) == 1 and isinstance(paragraph[0], nodes.paragraph):
        inner = paragraph[0]
        paragraph.remove(inner)
        paragraph.extend(inner.children)
    parent += paragraph


def _build_region_table(region: dict[str, Any]) -> nodes.table:
    table = nodes.table()
    table["classes"] = ["memory-layout-table"]

    tgroup = nodes.tgroup(cols=3)
    table += tgroup
    tgroup.extend([nodes.colspec(colwidth=24), nodes.colspec(colwidth=12), nodes.colspec(colwidth=12)])

    thead = nodes.thead()
    tgroup += thead
    header_row = nodes.row()
    thead += header_row
    for title in ("Partition", "Offset", "Size"):
        entry = nodes.entry()
        entry += nodes.Text(title)
        header_row += entry

    tbody = nodes.tbody()
    tgroup += tbody
    for label, offset_bytes, size_bytes in _iter_partition_rows(region.get("partitions") or []):
        row = nodes.row()
        tbody += row

        label_entry = nodes.entry()
        label_entry += nodes.Text(label)
        row += label_entry

        offset_entry = nodes.entry()
        offset_entry += nodes.Text(_format_offset(offset_bytes))
        row += offset_entry

        size_entry = nodes.entry()
        size_entry += nodes.Text(_format_size(size_bytes))
        row += size_entry

    return table


def build_layout_table_nodes(directive: SphinxDirective, data: dict[str, Any]) -> nodes.Element:
    wrapper = nodes.container()
    wrapper["classes"] = ["memory-layout-table-board"]

    regions = data.get("reference_regions") or []
    for index, region in enumerate(regions):
        if index:
            wrapper += nodes.raw("", '<div class="memory-layout-table-region-spacer"></div>', format="html")

        total_bytes = int(region["total_bytes"])
        title = nodes.paragraph()
        title["classes"] = ["memory-layout-table-region-title"]
        emphasis = nodes.strong()
        emphasis += nodes.Text(
            f'{region["title"]} (size: {_hex_label(total_bytes)} = {_bytes_label(total_bytes)})'
        )
        title += emphasis
        wrapper += title

        if region.get("address_note_rst"):
            _append_rst_paragraph(directive.state, wrapper, str(region["address_note_rst"]))

        wrapper += _build_region_table(region)

    return wrapper


def _render_board_layout_html(data: dict[str, Any]) -> str:
    regions = "".join(_render_region_html(region) for region in data.get("reference_regions", []))
    board_name = data.get("board", {}).get("name", "")
    return (
        f'<div class="memory-layout-board" '
        f'data-board="{html.escape(board_name, quote=True)}">'
        f"{regions}</div>"
    )


class MemoryLayoutBoard(SphinxDirective):
    """Render reference memory layout charts from docs/data/memory/<board>.yaml."""

    required_arguments = 0
    optional_arguments = 0
    final_argument_whitespace = True
    has_content = False
    option_spec = {
        "board": directives.unchanged_required,
    }

    def run(self) -> list[nodes.Node]:
        data = load_board_data(self.options["board"])
        if not data.get("reference_regions"):
            note = nodes.paragraph()
            note += nodes.emphasis(text="No memory layout data found.")
            return [note]
        container = nodes.container()
        container += nodes.raw("", _render_board_layout_html(data), format="html")
        return [container]


class MemoryLayoutTable(SphinxDirective):
    """Render reference memory layout tables from docs/data/memory/<board>.yaml."""

    required_arguments = 0
    optional_arguments = 0
    final_argument_whitespace = True
    has_content = False
    option_spec = {
        "board": directives.unchanged_required,
    }

    def run(self) -> list[nodes.Node]:
        data = load_board_data(self.options["board"])
        if not data.get("reference_regions"):
            note = nodes.paragraph()
            note += nodes.emphasis(text="No memory layout data found.")
            return [note]
        return [build_layout_table_nodes(self, data)]


def add_memory_layout_viz_resources(app: Sphinx) -> None:
    static_path = RESOURCES_DIR.as_posix()
    if static_path not in app.config.html_static_path:
        app.config.html_static_path.append(static_path)
    app.add_css_file("memory_layout.css")


def setup(app: Sphinx) -> dict[str, Any]:
    app.add_directive("memory-layout-board", MemoryLayoutBoard)
    app.add_directive("memory-layout-table", MemoryLayoutTable)
    app.connect("builder-inited", add_memory_layout_viz_resources)
    return {
        "version": __version__,
        "parallel_read_safe": True,
        "parallel_write_safe": True,
    }
