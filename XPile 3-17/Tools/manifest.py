#!/usr/bin/env python3
"""manifest.py — Read/write ul.toml project manifests."""

import os

MANIFEST_NAME = "ul.toml"

TEMPLATE = """\
[project]
name = "{name}"
version = "0.1.0"
description = ""
entry = "main.ul"

[dependencies]

[build]
target = "native"
dr_memory = "gc"
"""


class Manifest:
    def __init__(self, path=None):
        self.path = path or MANIFEST_NAME
        self.project = {"name": "", "version": "0.1.0", "description": "", "entry": "main.ul"}
        self.dependencies = {}   # name → version string
        self.build = {"target": "native", "dr_memory": "gc"}

    @classmethod
    def load(cls, directory="."):
        m = cls(os.path.join(directory, MANIFEST_NAME))
        if not os.path.isfile(m.path):
            raise FileNotFoundError(f"{MANIFEST_NAME} not found in {os.path.abspath(directory)}")
        text = open(m.path, encoding="utf-8").read()
        m._parse(text)
        return m

    @classmethod
    def create(cls, directory, name):
        m = cls(os.path.join(directory, MANIFEST_NAME))
        m.project["name"] = name
        m.save()
        return m

    def save(self):
        lines = []
        lines.append("[project]")
        for k, v in self.project.items():
            lines.append(f'{k} = "{v}"')
        lines.append("")
        lines.append("[dependencies]")
        for name, ver in sorted(self.dependencies.items()):
            lines.append(f'{name} = "{ver}"')
        lines.append("")
        lines.append("[build]")
        for k, v in self.build.items():
            lines.append(f'{k} = "{v}"')
        lines.append("")
        with open(self.path, "w", encoding="utf-8") as f:
            f.write("\n".join(lines))

    def add_dependency(self, name, version):
        self.dependencies[name] = version
        self.save()

    def remove_dependency(self, name):
        if name in self.dependencies:
            del self.dependencies[name]
            self.save()
            return True
        return False

    def _parse(self, text):
        """Simple TOML parser — handles [section] and key = "value" lines."""
        section = None
        for line in text.splitlines():
            line = line.strip()
            if not line or line.startswith("#"):
                continue
            if line.startswith("[") and line.endswith("]"):
                section = line[1:-1].strip()
                continue
            if "=" in line:
                key, _, val = line.partition("=")
                key = key.strip()
                val = val.strip().strip('"').strip("'")
                if section == "project":
                    self.project[key] = val
                elif section == "dependencies":
                    self.dependencies[key] = val
                elif section == "build":
                    self.build[key] = val
