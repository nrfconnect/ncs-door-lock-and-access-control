"""
Copyright (c) 2026 Nordic Semiconductor ASA

SPDX-License-Identifier: LicenseRef-Nordic-5-Clause

Shared memory requirement data for documentation tables and charts.

Board data lives in docs/data/memory/<board>.yaml.
"""

from __future__ import annotations

from pathlib import Path
from typing import Any

import yaml
from docutils import nodes
from docutils.parsers.rst import directives
from docutils.statemachine import StringList
from sphinx.application import Sphinx
from sphinx.util.docutils import SphinxDirective

__version__ = "0.1.0"

RESOURCES_DIR = Path(__file__).parent / "static"
DATA_DIR = Path(__file__).parents[1] / "data" / "memory"

INTERNAL_NVM_COLUMNS = (
    ("boot", "MCUboot"),
    ("slot0", "Application (used / free)"),
    ("factory_data", "Factory data"),
    ("storage", "Settings"),
)

EXTERNAL_NVM_COLUMN_ORDER = ("slot1",)

RAM_SUBHEAD = "used / free"
EMPTY_CELL = "--"
PADDING_LABEL = "Unused"

STACK_SUBHEADS = ("stack usage", "stack size")


def applicable_samples(data: dict[str, Any]) -> list[dict[str, Any]]:
    return [sample for sample in (data.get("samples") or []) if not sample.get("not_applicable")]


def resolve_stack_threads(data: dict[str, Any]) -> dict[str, dict[str, str]]:
    """Return configured stack threads from board YAML (see stack_threads in memory_matter.yaml)."""
    threads = data.get("stack_threads")
    if not threads:
        return {}
    resolved: dict[str, dict[str, str]] = {}
    for thread_id, info in threads.items():
        entry = {"title": str(info.get("title", thread_id))}
        if info.get("kconfig"):
            entry["kconfig"] = str(info["kconfig"])
        resolved[thread_id] = entry
    return resolved


def stack_thread_ids(data: dict[str, Any]) -> list[str]:
    return list(resolve_stack_threads(data).keys())


def stack_samples(data: dict[str, Any]) -> list[dict[str, Any]]:
    thread_ids = stack_thread_ids(data)
    if not thread_ids:
        return []
    samples: list[dict[str, Any]] = []
    for sample in data.get("samples") or []:
        stack = sample.get("stack")
        if stack and all(thread_id in stack for thread_id in thread_ids):
            samples.append(sample)
    return samples


def stack_thread_max_sizes(samples: list[dict[str, Any]], thread_ids: list[str]) -> dict[str, int]:
    sizes: dict[str, int] = {}
    for thread_id in thread_ids:
        sizes[thread_id] = max(int(sample["stack"][thread_id]["size_b"]) for sample in samples)
    return sizes


def sample_stack_table_cells(sample: dict[str, Any], thread_ids: list[str]) -> list[str]:
    stack = sample["stack"]
    cells: list[str] = []
    for thread_id in thread_ids:
        cells.append(str(int(stack[thread_id]["used_b"])))
        cells.append(str(int(stack[thread_id]["size_b"])))
    return cells


def load_board_data(board: str) -> dict[str, Any]:
    path = DATA_DIR / f"{board}.yaml"
    if not path.is_file():
        raise FileNotFoundError(f"Memory data file not found: {path}")
    with path.open(encoding="utf-8") as handle:
        return yaml.safe_load(handle)


def _part_size_bytes(part: dict[str, Any]) -> int:
    size_kb = float(part["size_kb"])
    return round(size_kb * 1024)


def _padding_part(offset: int, size_bytes: int) -> dict[str, Any]:
    return {
        "id": "padding",
        "label": PADDING_LABEL,
        "size_kb": round(size_bytes / 1024),
        "offset": offset,
    }


def _sort_partitions(partitions: list[dict[str, Any]]) -> list[dict[str, Any]]:
    real = [part for part in partitions if part.get("id") != "padding"]
    if real and all("offset" in part for part in real):
        return sorted(partitions, key=lambda part: (part.get("offset", 0), part["order"]))
    return sorted(partitions, key=lambda part: part["order"])


def expand_partition_gaps(
    partitions: list[dict[str, Any]],
    flash_total_kb: float | None,
) -> list[dict[str, Any]]:
    real = [
        part
        for part in partitions
        if part.get("id") != "padding" and part.get("memory") != "ram"
    ]
    if not real:
        return []

    if all("offset" in part for part in real):
        ordered = sorted(real, key=lambda part: part["offset"])
        expanded: list[dict[str, Any]] = []
        cursor = 0
        flash_end = round(float(flash_total_kb) * 1024) if flash_total_kb is not None else None

        for part in ordered:
            offset = int(part["offset"])
            if offset > cursor:
                gap_bytes = offset - cursor
                if gap_bytes > 0:
                    expanded.append(_padding_part(cursor, gap_bytes))
            expanded.append(dict(part))
            cursor = max(cursor, offset + _part_size_bytes(part))

        if flash_end is not None and cursor < flash_end:
            gap_bytes = flash_end - cursor
            if gap_bytes > 0:
                expanded.append(_padding_part(cursor, gap_bytes))

        for order, part in enumerate(expanded):
            part["order"] = order
        return expanded

    ordered = sorted(real, key=lambda part: part["order"])
    if flash_total_kb is None:
        return ordered

    used_kb = sum(float(part["size_kb"]) for part in ordered)
    if flash_total_kb <= used_kb:
        return ordered

    padding_kb = flash_total_kb - used_kb
    expanded = list(ordered)
    expanded.append(
        {
            "id": "padding",
            "label": PADDING_LABEL,
            "size_kb": padding_kb,
            "order": len(expanded),
        }
    )
    return expanded


def resolve_layout_raw(data: dict[str, Any], layout_name: str) -> dict[str, Any]:
    layouts = data["layouts"]
    layout = layouts[layout_name]
    if "extends" in layout:
        base = resolve_layout_raw(data, layout["extends"])
        own = layout.get("partitions", [])
        own_ids = {part["id"] for part in own}

        merged: dict[str, dict[str, Any]] = {part["id"]: dict(part) for part in own}
        for part in base["partitions"]:
            if part["id"] not in own_ids and part["id"] != "padding":
                merged[part["id"]] = dict(part)

        partitions = _sort_partitions(list(merged.values()))
        external = layout.get("external", base.get("external"))
        return {"partitions": partitions, "external": external}
    return {
        "partitions": _sort_partitions(list(layout["partitions"])),
        "external": layout.get("external"),
    }


def _apply_layout_gaps(data: dict[str, Any], layout: dict[str, Any]) -> dict[str, Any]:
    partitions = expand_partition_gaps(
        layout["partitions"],
        board_nvm_total_kb(data),
    )
    external = layout.get("external")
    if external is not None:
        external = {
            **external,
            "partitions": expand_partition_gaps(
                external["partitions"],
                float(external["nvm_total_kb"]),
            ),
        }
    return {"partitions": partitions, "external": external}


def resolve_layout(data: dict[str, Any], layout_name: str) -> dict[str, Any]:
    return _apply_layout_gaps(data, resolve_layout_raw(data, layout_name))


def layout_nvm_total_kb(layout: dict[str, Any]) -> float:
    real = [
        part
        for part in layout["partitions"]
        if part.get("memory") != "ram" and part.get("id") != "padding"
    ]
    if real and all("offset" in part for part in real):
        end = max(int(part["offset"]) + _part_size_bytes(part) for part in real)
        return end / 1024
    return float(sum(part["size_kb"] for part in real))


def board_nvm_total_kb(data: dict[str, Any]) -> float:
    board_total = _optional_kb(data.get("board", {}).get("nvm_total_kb"))
    if board_total is not None:
        return board_total
    layouts = data.get("layouts") or {}
    default_layout = "release" if "release" in layouts else next(iter(layouts), None)
    if default_layout is None:
        raise ValueError(f"No layouts defined for board {data.get('board', {}).get('name', 'unknown')}")
    return layout_nvm_total_kb(resolve_layout_raw(data, default_layout))


def board_ram_total_kb(data: dict[str, Any]) -> float | None:
    board_total = _optional_kb(data.get("board", {}).get("ram_total_kb"))
    if board_total is not None:
        return board_total
    for region in data.get("reference_regions", []):
        region_id = str(region.get("id", ""))
        title = str(region.get("title", ""))
        if "sram" in region_id or "ram" in title.lower():
            total_bytes = region.get("total_bytes")
            if total_bytes is not None:
                return float(total_bytes) / 1024
    return None


def _optional_kb(value: Any) -> float | None:
    if value is None or value == "None":
        return None
    return float(value)


def sample_layout(data: dict[str, Any], sample: dict[str, Any]) -> dict[str, Any] | None:
    layout_name = sample.get("layout")
    if layout_name is None:
        return None
    return resolve_layout(data, layout_name)


def board_external_nvm_columns(data: dict[str, Any]) -> list[tuple[str, str]]:
    labels: dict[str, str] = {}
    order: dict[str, int] = {}
    for layout in data.get("layouts", {}).values():
        external = layout.get("external")
        if not external:
            continue
        for part in external["partitions"]:
            if part["id"] == "padding":
                continue
            labels.setdefault(part["id"], part["label"])
            order.setdefault(part["id"], int(part["order"]))

    if not labels:
        return []

    column_order = {part_id: index for index, part_id in enumerate(EXTERNAL_NVM_COLUMN_ORDER)}
    return sorted(
        labels.items(),
        key=lambda item: (column_order.get(item[0], len(EXTERNAL_NVM_COLUMN_ORDER)), order[item[0]]),
    )


def board_has_external_nvm(data: dict[str, Any]) -> bool:
    return bool(board_external_nvm_columns(data))


def board_external_total_kb(data: dict[str, Any]) -> float | None:
    for layout in data.get("layouts", {}).values():
        external = layout.get("external")
        if external:
            return float(external["nvm_total_kb"])
    return None


def _partition_map(layout: dict[str, Any] | None) -> dict[str, dict[str, Any]]:
    if layout is None:
        return {}
    parts = {
        part["id"]: part for part in layout["partitions"] if part.get("id") != "padding"
    }
    external = layout.get("external")
    if external:
        for part in external["partitions"]:
            if part.get("id") != "padding":
                parts[part["id"]] = part
    return parts


def _fmt_kb(value: int | float) -> str:
    if value == int(value):
        return str(int(value))
    return f"{value:.1f}"


def _used_free(used: int | float, total: int | float | None) -> str:
    if total is None:
        return _fmt_kb(used)
    free = max(float(total) - float(used), 0)
    return f"{_fmt_kb(used)} / {_fmt_kb(free)}"


def sample_table_cells(
    data: dict[str, Any],
    sample: dict[str, Any],
    *,
    include_external: bool,
) -> list[str]:
    usage = sample.get("usage", {})
    nvm = usage.get("nvm", {})
    layout = sample_layout(data, sample)
    parts = _partition_map(layout)
    ram_total = board_ram_total_kb(data)

    cells: list[str] = []

    for part_id, _, used_free in (
        ("boot", "", False),
        ("slot0", "", True),
        ("factory_data", "", False),
        ("storage", "", False),
    ):
        part = parts.get(part_id)
        used = nvm.get(part_id)
        if part is None and used is None:
            cells.append(EMPTY_CELL)
            continue
        size_kb = float(part["size_kb"]) if part is not None else None
        if used_free and size_kb is not None:
            cells.append(_used_free(float(used or 0), size_kb))
        elif size_kb is not None:
            cells.append(_fmt_kb(size_kb))
        elif used is not None:
            cells.append(_fmt_kb(used))
        else:
            cells.append(EMPTY_CELL)

    external_usage = usage.get("external", {})
    if include_external:
        for part_id, _ in board_external_nvm_columns(data):
            part = parts.get(part_id)
            ext_size = float(part["size_kb"]) if part is not None else None
            ext_used = external_usage.get(part_id)
            if ext_used is not None and ext_size is not None:
                cells.append(_used_free(float(ext_used), ext_size))
            elif ext_size is not None:
                cells.append(_fmt_kb(ext_size))
            else:
                cells.append(EMPTY_CELL)

    ram_used = float(usage.get("ram_used_kb", 0))
    cells.append(_used_free(ram_used, float(ram_total) if ram_total is not None else None))
    return cells


def _header_label(data: dict[str, Any], kind: str) -> str:
    board = data.get("board", {})
    if kind == "nvm":
        total = board_nvm_total_kb(data)
        return f"Internal NVM ({_fmt_kb(total)} kB)"
    if kind == "external":
        for layout in data.get("layouts", {}).values():
            external = layout.get("external")
            if external:
                return f"External NVM ({_fmt_kb(external['nvm_total_kb'])} kB)"
        return "External NVM"
    ram_total = board.get("ram_total_kb")
    resolved = _optional_kb(ram_total) if "ram_total_kb" in board else board_ram_total_kb(data)
    return f"RAM ({_fmt_kb(resolved)} kB)" if resolved is not None else "RAM"


def _append_rst_cell(state, entry: nodes.entry, rst_text: str) -> None:
    paragraph = nodes.paragraph()
    content = StringList([rst_text], "<memory_table>")
    state.nested_parse(content, 0, paragraph)
    if len(paragraph) == 1 and isinstance(paragraph[0], nodes.paragraph):
        inner = paragraph[0]
        paragraph.remove(inner)
        paragraph.extend(inner.children)
    entry += paragraph


def _append_text_cell(row: nodes.row, text: str, *, css_class: str = "memory-req-value") -> None:
    entry = nodes.entry()
    entry["classes"] = [css_class]
    entry += nodes.Text(text)
    row += entry


def build_memory_table_nodes(directive: SphinxDirective, data: dict[str, Any]) -> nodes.table:
    external_columns = board_external_nvm_columns(data)
    include_external = bool(external_columns)
    internal_count = len(INTERNAL_NVM_COLUMNS)
    external_count = len(external_columns)
    total_cols = 1 + internal_count + external_count + 1

    table = nodes.table()
    table["classes"] = ["memory-req-table"]

    tgroup = nodes.tgroup(cols=total_cols)
    table += tgroup
    tgroup.extend([nodes.colspec(colwidth=28)] + [nodes.colspec(colwidth=10)] * (total_cols - 1))

    thead = nodes.thead()
    tgroup += thead

    group_row = nodes.row()
    thead += group_row

    sample_group = nodes.entry()
    sample_group["morerows"] = 1
    sample_group += nodes.Text("Application")
    group_row += sample_group

    nvm_group = nodes.entry()
    nvm_group["morecols"] = internal_count - 1
    nvm_group["classes"] = ["memory-req-group-nvm"]
    nvm_group += nodes.Text(_header_label(data, "nvm"))
    group_row += nvm_group

    if include_external:
        external_group = nodes.entry()
        if external_count > 1:
            external_group["morecols"] = external_count - 1
        external_group["classes"] = ["memory-req-group-external"]
        external_group += nodes.Text(_header_label(data, "external"))
        group_row += external_group

    ram_group = nodes.entry()
    ram_group["classes"] = ["memory-req-group-ram"]
    ram_group += nodes.Text(_header_label(data, "ram"))
    group_row += ram_group

    sub_row = nodes.row()
    thead += sub_row

    for _, label in INTERNAL_NVM_COLUMNS:
        entry = nodes.entry()
        entry["classes"] = ["memory-req-subhead"]
        entry += nodes.Text(label)
        sub_row += entry

    for part_id, label in external_columns:
        entry = nodes.entry()
        entry["classes"] = ["memory-req-subhead"]
        if part_id == "slot1":
            label = f"{label} (used / free)"
        entry += nodes.Text(label)
        sub_row += entry

    ram_sub = nodes.entry()
    ram_sub["classes"] = ["memory-req-subhead"]
    ram_sub += nodes.Text(RAM_SUBHEAD)
    sub_row += ram_sub

    tbody = nodes.tbody()
    tgroup += tbody

    for sample in applicable_samples(data):
        row = nodes.row()
        tbody += row

        sample_entry = nodes.entry()
        sample_entry["classes"] = ["memory-req-sample"]
        _append_rst_cell(directive.state, sample_entry, sample["label"])
        row += sample_entry

        cells = sample_table_cells(data, sample, include_external=include_external)
        for value in cells:
            css = "memory-req-empty" if value == EMPTY_CELL else "memory-req-value"
            _append_text_cell(row, value, css_class=css)

    return table


def build_ram_table_nodes(directive: SphinxDirective, data: dict[str, Any]) -> nodes.table:
    table = nodes.table()
    table["classes"] = ["memory-req-table", "memory-req-table-ram-only"]

    tgroup = nodes.tgroup(cols=2)
    table += tgroup
    tgroup.extend([nodes.colspec(colwidth=28), nodes.colspec(colwidth=12)])

    thead = nodes.thead()
    tgroup += thead

    header_row = nodes.row()
    thead += header_row

    sample_header = nodes.entry()
    sample_header["morerows"] = 1
    sample_header += nodes.Text("Application")
    header_row += sample_header

    ram_header = nodes.entry()
    ram_header["classes"] = ["memory-req-group-ram"]
    ram_header += nodes.Text(_header_label(data, "ram"))
    header_row += ram_header

    sub_row = nodes.row()
    thead += sub_row

    ram_sub = nodes.entry()
    ram_sub["classes"] = ["memory-req-subhead"]
    ram_sub += nodes.Text(RAM_SUBHEAD)
    sub_row += ram_sub

    tbody = nodes.tbody()
    tgroup += tbody

    ram_total = data.get("board", {}).get("ram_total_kb")
    for sample in applicable_samples(data):
        row = nodes.row()
        tbody += row

        sample_entry = nodes.entry()
        sample_entry["classes"] = ["memory-req-sample"]
        _append_rst_cell(directive.state, sample_entry, sample["label"])
        row += sample_entry

        usage = sample.get("usage", {})
        ram_used = float(usage.get("ram_used_kb", 0))
        _append_text_cell(
            row,
            _used_free(ram_used, float(ram_total) if ram_total is not None else None),
        )

    return table


def build_stack_table_nodes(directive: SphinxDirective, data: dict[str, Any]) -> nodes.Element:
    samples = stack_samples(data)
    if not samples:
        board_name = data.get("board", {}).get("name", "unknown")
        paragraph = nodes.paragraph()
        paragraph += nodes.Text(f"No stack measurements for board {board_name}.")
        return paragraph

    threads = resolve_stack_threads(data)
    thread_ids = list(threads.keys())
    cols = 1 + len(thread_ids) * 2

    table = nodes.table()
    table["classes"] = ["memory-req-table", "stack-req-table"]

    tgroup = nodes.tgroup(cols=cols)
    table += tgroup
    tgroup.extend([nodes.colspec(colwidth=28)] + [nodes.colspec(colwidth=10)] * (cols - 1))

    thead = nodes.thead()
    tgroup += thead

    group_row = nodes.row()
    thead += group_row

    sample_group = nodes.entry()
    sample_group["morerows"] = 1
    sample_group += nodes.Text("Application")
    group_row += sample_group

    for thread_id in thread_ids:
        thread_group = nodes.entry()
        thread_group["morecols"] = 1
        thread_group["classes"] = ["memory-req-group-stack", f"memory-req-group-{thread_id}"]
        thread_group += nodes.Text(threads[thread_id]["title"])
        group_row += thread_group

    sub_row = nodes.row()
    thead += sub_row

    for _thread_id in thread_ids:
        for label in STACK_SUBHEADS:
            entry = nodes.entry()
            entry["classes"] = ["memory-req-subhead"]
            entry += nodes.Text(label)
            sub_row += entry

    tbody = nodes.tbody()
    tgroup += tbody

    for sample in samples:
        row = nodes.row()
        tbody += row

        sample_entry = nodes.entry()
        sample_entry["classes"] = ["memory-req-sample"]
        _append_rst_cell(directive.state, sample_entry, sample["label"])
        row += sample_entry

        for value in sample_stack_table_cells(sample, thread_ids):
            _append_text_cell(row, value)

    return table


class StackTable(SphinxDirective):
    """Render stack usage table from docs/data/memory/."""

    required_arguments = 0
    optional_arguments = 0
    final_argument_whitespace = True
    has_content = False
    option_spec = {
        "board": directives.unchanged_required,
    }

    def run(self) -> list[nodes.Node]:
        data = load_board_data(self.options["board"])
        samples = stack_samples(data)
        if not samples:
            board_name = data.get("board", {}).get("name", self.options["board"])
            paragraph = nodes.paragraph()
            paragraph += nodes.Text(f"No stack measurements for board {board_name}.")
            return [paragraph]
        return [build_stack_table_nodes(self, data)]


class MemoryTable(SphinxDirective):
    """Render memory requirements table from docs/data/memory/."""

    required_arguments = 0
    optional_arguments = 0
    final_argument_whitespace = True
    has_content = False
    option_spec = {
        "board": directives.unchanged_required,
    }

    def run(self) -> list[nodes.Node]:
        data = load_board_data(self.options["board"])
        return [build_memory_table_nodes(self, data)]


class RamTable(SphinxDirective):
    """Render static RAM usage table from docs/data/memory/."""

    required_arguments = 0
    optional_arguments = 0
    final_argument_whitespace = True
    has_content = False
    option_spec = {
        "board": directives.unchanged_required,
    }

    def run(self) -> list[nodes.Node]:
        data = load_board_data(self.options["board"])
        return [build_ram_table_nodes(self, data)]


def add_memory_table_resources(app: Sphinx) -> None:
    static_path = RESOURCES_DIR.as_posix()
    if static_path not in app.config.html_static_path:
        app.config.html_static_path.append(static_path)
    app.add_css_file("memory_table.css")


def setup(app: Sphinx) -> dict[str, Any]:
    app.add_directive("memory-table", MemoryTable)
    app.add_directive("ram-table", RamTable)
    app.add_directive("stack-table", StackTable)
    app.connect("builder-inited", add_memory_table_resources)
    return {
        "version": __version__,
        "parallel_read_safe": True,
        "parallel_write_safe": True,
    }
