# Copyright (c) 2026 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: Apache-2.0

"""
Release filter for the known issues page.
"""

import html
import json
import re
from pathlib import Path
from typing import Any

from docutils import nodes
from docutils.parsers.rst import directives
from sphinx.util.docutils import SphinxDirective

__version__ = "0.1.0"

_VERSION_PATTERN = re.compile(r"^\d+\.\d+\.\d+$")
_DEVELOPMENT_VERSION_PATTERN = re.compile(r"^\d+\.\d+\.99$")


def _version_class(version: str) -> str:
    return f"v{version.replace('.', '-')}"


class KnownIssuesFilter(SphinxDirective):
    has_content = False
    option_spec = {"default": directives.unchanged}

    def run(self) -> list[nodes.Node]:
        versions_file = Path(self.env.srcdir) / "versions.json"
        versions = json.loads(versions_file.read_text(encoding="utf-8"))
        versions = [
            version
            for version in versions
            if _VERSION_PATTERN.fullmatch(version)
            and not _DEVELOPMENT_VERSION_PATTERN.fullmatch(version)
        ]

        if not versions:
            raise self.error(
                f'No release versions found in "{versions_file}".')

        default = self.options.get("default", versions[0])
        if default not in versions:
            raise self.error(
                f'Default release "{default}" is not in "{versions_file}".'
            )

        options = ['<option value="all">All releases</option>']
        for version in versions:
            selected = " selected" if version == default else ""
            options.append(
                f'<option value="{_version_class(version)}"{selected}>'
                f"v{html.escape(version)}</option>"
            )

        filter_html = f"""
<div class="known-issues-filter">
  <label for="known-issues-release">Release:</label>
  <select id="known-issues-release">
    {"".join(options)}
  </select>
</div>
<style>
.known-issues-filter {{
  align-items: center;
  display: flex;
  gap: 0.5rem;
  margin: 1rem 0 1.5rem;
}}
.known-issues-filter label {{
  font-weight: 700;
}}
.known-issues-filter select {{
  min-width: 9rem;
  padding: 0.35rem 2rem 0.35rem 0.5rem;
}}
.known-issue-filter-hidden {{
  display: none !important;
}}
.rst-content section#known-issues dl > dt {{
  background-color: #e7f2fa !important;
  border-top: 3px solid #6ab0de;
  color: #000;
  display: block;
  font-size: 90%;
  font-weight: 700;
  line-height: normal;
  margin: 6px 0;
  padding: 6px;
}}
.known-issue-version {{
  border: 1px solid #e97c25;
  color: #e97c25;
  display: inline-block;
  font-size: 0.8em;
  margin-left: 0.5rem;
  padding: 0 0.25rem;
}}
.known-issue-wontfix {{
  border: 1px solid red;
  color: red;
  display: inline-block;
  font-size: 0.8em;
  margin-left: 0.5rem;
  padding: 0 0.25rem;
}}
</style>
<script>
document.addEventListener("DOMContentLoaded", function () {{
  const filter = document.getElementById("known-issues-release");
  const versionPattern = /^v\\d+-\\d+-\\d+$/;
  const issues = Array.from(document.querySelectorAll("dl")).filter(function (issue) {{
    return Array.from(issue.classList).some(function (name) {{
      return versionPattern.test(name);
    }});
  }});

  issues.forEach(function (issue) {{
    const title = issue.querySelector("dt");
    if (!title) return;

    if (issue.classList.contains("wontfix")) {{
      const tag = document.createElement("span");
      tag.className = "known-issue-wontfix";
      tag.textContent = "Won't fix";
      title.appendChild(tag);
    }}

    Array.from(issue.classList)
      .filter(function (name) {{ return versionPattern.test(name); }})
      .forEach(function (name) {{
        const tag = document.createElement("span");
        tag.className = "known-issue-version";
        tag.textContent = name.replace(/^v/, "v").replaceAll("-", ".");
        title.appendChild(tag);
      }});
  }});

  function applyFilter() {{
    issues.forEach(function (issue) {{
      const hidden =
        filter.value !== "all" && !issue.classList.contains(filter.value);
      issue.classList.toggle("known-issue-filter-hidden", hidden);
    }});
  }}

  filter.addEventListener("change", applyFilter);
  applyFilter();
}});
</script>
"""
        return [nodes.raw("", filter_html, format="html")]


def setup(app) -> dict[str, Any]:
    app.add_directive("known-issues-filter", KnownIssuesFilter)

    return {
        "version": __version__,
        "parallel_read_safe": True,
        "parallel_write_safe": True,
    }
