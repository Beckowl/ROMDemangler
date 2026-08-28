import sys
import os
from PyQt6.QtWidgets import (
    QApplication, QWidget, QVBoxLayout, QHBoxLayout, QCheckBox,
    QPushButton, QLabel, QGroupBox, QGridLayout, QRadioButton,
    QButtonGroup, QPlainTextEdit, QFileDialog
)
from PyQt6.QtCore import Qt, QProcess
from PyQt6.QtGui import QDragEnterEvent, QDropEvent, QFontDatabase, QFont, QIcon

Num2Name = {
    4: 'bbh',
    5: 'ccm',
    6: 'castle_inside',
    7: 'hmc',
    8: 'ssl',
    9: 'bob',
    10: 'sl',
    11: 'wdw',
    12: 'jrb',
    13: 'thi',
    14: 'ttc',
    15: 'rr',
    16: 'castle_grounds',
    17: 'bitdw',
    18: 'vcutm',
    19: 'bitfs',
    20: 'sa',
    21: 'bits',
    22: 'lll',
    23: 'ddd',
    24: 'wf',
    # 25: 'ending',
    26: 'castle_courtyard',
    27: 'pss',
    28: 'cotmc',
    29: 'totwc',
    30: 'bowser_1',
    31: 'wmotr',
    33: 'bowser_2',
    34: 'bowser_3',
    36: 'ttm'
}


class RomOpener(QWidget):
    def __init__(self):
        super().__init__()

        self.rom_path = ""

        layout = QHBoxLayout()
        self.setLayout(layout)

        self.open_btn = QPushButton("Open ROM")
        self.rom_label = QLabel("No ROM loaded")

        layout.addWidget(self.open_btn)
        layout.addWidget(self.rom_label)

        self.open_btn.clicked.connect(self.open_rom)

    def open_rom(self):
        file_path, _ = QFileDialog.getOpenFileName(
            self,
            "Open ROM",
            "",
            "ROM Files (*.z64)"
        )

        if file_path:
            self.rom_path = file_path
            self.rom_label.setText(os.path.basename(file_path))


class RamOpener(QWidget):
    def __init__(self):
        super().__init__()

        self.ram_path = ""

        layout = QHBoxLayout()
        self.setLayout(layout)

        self.open_btn = QPushButton("Open RAM Dump")
        self.ram_label = QLabel("No RAM dump loaded")

        layout.addWidget(self.open_btn)
        layout.addWidget(self.ram_label)

        self.open_btn.clicked.connect(self.open_ram)

    def open_ram(self):
        file_path, _ = QFileDialog.getOpenFileName(
            self,
            "Open RAM Dump",
            "",
            "RAM Dumps (*.bin);;Binary Files (*.bin);;All Files (*)"
        )

        if file_path:
            self.ram_path = file_path
            self.ram_label.setText(os.path.basename(file_path))


class DemanglerGUI(QWidget):
    def __init__(self):
        super().__init__()

        self.setWindowTitle("ROM Demangler")
        self.setMinimumSize(450, 550)
        self.setMaximumSize(450, 550)

        layout = QVBoxLayout()
        self.setLayout(layout)

        top_layout = QVBoxLayout()

        self.rom_drop = RomOpener()
        top_layout.addWidget(self.rom_drop)

        self.ram_drop = RamOpener()
        top_layout.addWidget(self.ram_drop)

        layout.addLayout(top_layout)

        export_group = QGroupBox("Export Options")
        export_layout = QGridLayout()

        self.checks = {}

        opts = [
            "actors",
            "sounds",
            "tweaks",
            "fix collision",
            "ignore segment 0"
        ]

        for i, opt in enumerate(opts):
            cb = QCheckBox(opt.capitalize())
            self.checks[opt] = cb
            export_layout.addWidget(cb, i // 2, i % 2)

        export_group.setLayout(export_layout)
        layout.addWidget(export_group)

        level_group = QGroupBox("Levels")
        level_layout = QVBoxLayout()

        level_buttons_layout = QHBoxLayout()

        select_all = QPushButton("Select All")
        unselect_all = QPushButton("Unselect All")

        level_buttons_layout.addStretch()
        level_buttons_layout.addWidget(select_all)
        level_buttons_layout.addWidget(unselect_all)

        level_layout.addLayout(level_buttons_layout)

        grid = QGridLayout()

        self.level_checkboxes = []

        for idx, (num, name) in enumerate(Num2Name.items()):
            cb = QCheckBox(name)
            self.level_checkboxes.append((cb, num))
            grid.addWidget(cb, idx // 4, idx % 4)

        level_layout.addLayout(grid)

        level_group.setLayout(level_layout)
        layout.addWidget(level_group)

        select_all.clicked.connect(
            lambda: [cb.setChecked(True) for cb, _ in self.level_checkboxes]
        )

        unselect_all.clicked.connect(
            lambda: [cb.setChecked(False) for cb, _ in self.level_checkboxes]
        )

        self.run_btn = QPushButton("Demangle")
        self.run_btn.setFixedHeight(40)

        layout.addWidget(self.run_btn)
        self.run_btn.clicked.connect(self.run)

        self.status_label = QLabel("")
        layout.addWidget(self.status_label)

        self.process = None

    def run(self):
        if (self.process and self.process.state() != QProcess.ProcessState.NotRunning):
            self.process.kill()
            self.status_label.setText("Cancelled.")
            self.run_btn.setText("Demangle")
            return

        rom = self.rom_drop.rom_path
        ram = self.ram_drop.ram_path

        if not rom:
            self.status_label.setText("Please select a ROM.")
            return

        args = [
            "--rom",
            rom
        ]

        if ram:
            args.extend([
                "--ram",
                ram
            ])

        levels = [
            str(lid)
            for cb, lid in self.level_checkboxes
            if cb.isChecked()
        ]

        if levels:
            args.extend([
                "--levels",
                ",".join(levels)
            ])
        if self.checks.get("actors") and self.checks["actors"].isChecked():
            args.extend([
                "--actors",
                "all"
            ])
        if self.checks.get("sounds") and self.checks["sounds"].isChecked():
            args.append("--sound")
        if self.checks.get("tweaks") and self.checks["tweaks"].isChecked():
            args.append("--tweaks")
        if (self.checks.get("fix collision") and self.checks["fix collision"].isChecked()):
            args.append("--fix-collision")
        if (self.checks.get("ignore segment 0") and self.checks["ignore segment 0"].isChecked()):
            args.append("--ignore-seg-0")

        self.status_label.setText("Running...")
        self.run_btn.setText("Cancel")

        print(args)

        self.process = QProcess(self)

        # piratesoftware moment :pray:
        if os.path.exists("ROMDemangler.exe"):
            program = "ROMDemangler.exe"
        elif os.path.exists("ROMDemangler"):
            program = "ROMDemangler"
        elif os.path.exists(os.path.join("build", "ROMDemangler.exe")):
            program = os.path.join("build", "ROMDemangler.exe")
        elif os.path.exists(os.path.join("build", "ROMDemangler")):
            program = os.path.join("build", "ROMDemangler")
        else:
            self.status_label.setText("ROMDemangler executable not found.")
            self.run_btn.setText("Demangle")
            return

        self.process.setProgram(program)
        self.process.setArguments(args)

        self.process.readyReadStandardOutput.connect(self.handle_stdout)
        self.process.readyReadStandardError.connect(self.handle_stderr)
        self.process.finished.connect(self.process_finished)

        self.process.start()

    def handle_stdout(self):
        data = self.process.readAllStandardOutput()
        print(bytes(data).decode("utf-8", errors="replace"))

    def handle_stderr(self):
        data = self.process.readAllStandardError()
        print(bytes(data).decode("utf-8", errors="replace"))

    def process_finished(self):
        if (self.process.exitStatus() == QProcess.ExitStatus.NormalExit and self.process.exitCode() == 0):
            self.status_label.setText("Done.")
        self.run_btn.setText("Demangle")

if __name__ == '__main__':
    app = QApplication(sys.argv)
    font_id = QFontDatabase.addApplicationFont("gui/sm64.ttf")
    app.setWindowIcon(QIcon("gui/icon.png"))
    if font_id == -1:
        print("Failed to load font!")
    else:
        families = QFontDatabase.applicationFontFamilies(font_id)
        if families:
            custom_font_family = families[0]
            font = QFont(custom_font_family, 10)
            app.setFont(font)
    window = DemanglerGUI()
    window.show()
    sys.exit(app.exec())