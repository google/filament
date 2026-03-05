import os
import sys
import shutil
import asyncio

from textual.app import App, ComposeResult
from textual.containers import Horizontal, Vertical, ScrollableContainer
from textual.widgets import Header, Footer, Button, Label, Static, ListItem, ListView, Input, DirectoryTree
from textual.screen import Screen, ModalScreen
from textual.reactive import reactive
from textual.message import Message

PACKAGE = "com.google.android.filament.validation"
EXTERNAL_DIR = f"/sdcard/Android/data/{PACKAGE}/files"
INTERNAL_DIR = "files"

if not shutil.which("adb"):
    print("Error: 'adb' not found in PATH. Please install Android Platform Tools and ensure it's in your PATH.", file=sys.stderr)
    sys.exit(1)

async def run_adb_cmd(*args):
    """Run an adb command asynchronously and return code, stdout, stderr."""
    proc = await asyncio.create_subprocess_exec(
        "adb", *args,
        stdout=asyncio.subprocess.PIPE,
        stderr=asyncio.subprocess.PIPE
    )
    stdout, stderr = await proc.communicate()
    return proc.returncode, stdout.decode().strip(), stderr.decode().strip()

class DeviceSelectScreen(Screen):
    def compose(self) -> ComposeResult:
        yield Label("Multiple Android devices detected. Please select one:", id="title")
        yield ListView(id="device_list")

    async def on_mount(self) -> None:
        code, out, err = await run_adb_cmd("devices")
        lv = self.query_one("#device_list", ListView)
        for line in out.splitlines()[1:]:
            if line.strip() and "device" in line:
                serial = line.split()[0]
                await lv.append(ListItem(Label(serial), id=f"dev_{serial}"))

    def on_list_view_selected(self, event: ListView.Selected) -> None:
        serial = event.item.id.replace("dev_", "")
        self.dismiss(serial)

class FileSelectScreen(ModalScreen[str]):
    CSS = """
    FileSelectScreen {
        align: center middle;
    }
    #dialog {
        width: 80%;
        height: 80%;
        padding: 1;
        border: thick $background 80%;
        background: $surface;
    }
    """
    def compose(self) -> ComposeResult:
        with Vertical(id="dialog"):
            yield Label("Select a local .zip test bundle to upload:")
            yield DirectoryTree(os.getcwd())
            with Horizontal():
                yield Button("Cancel", id="btn_cancel_upload", variant="error")

    def on_button_pressed(self, event: Button.Pressed) -> None:
        if event.button.id == "btn_cancel_upload":
            self.dismiss(None)

    def on_directory_tree_file_selected(self, event: DirectoryTree.FileSelected) -> None:
        if str(event.path).endswith(".zip"):
            self.dismiss(str(event.path))
        else:
            self.app.notify("Please select a .zip file!", severity="warning")

class FileItem(Static):
    class FileChanged(Message):
        """Emitted when a file is renamed or deleted."""
        pass
    class TestLoaded(Message):
        """Emitted when a test is loaded on device."""
        def __init__(self, filename: str) -> None:
            self.filename = filename
            super().__init__()

    def __init__(self, filename: str, filepath: str, is_internal: bool, serial: str, **kwargs):
        super().__init__(**kwargs)
        self.filename = filename
        self.filepath = filepath
        self.is_internal = is_internal
        self.serial = serial
        self.renaming = False

    def compose(self) -> ComposeResult:
        with Vertical(classes="file-item-container"):
            with Horizontal(id="file_row", classes="file-item-row"):
                yield Label(self.filename, id="lbl_filename", classes="file-name")

                # Only show Load button for test configurations, not results
                if not self.filename.startswith("results_"):
                    yield Button("▶", id="btn_load", variant="success", classes="compact-btn", tooltip="Load this test on device")

                yield Button("↓", id="btn_download", variant="primary", classes="compact-btn", tooltip="Download to PC")
                yield Button("✎", id="btn_start_rename", variant="warning", classes="compact-btn", tooltip="Rename on device")
                yield Button("✗", id="btn_delete", variant="error", classes="compact-btn", tooltip="Delete on device")
            with Horizontal(id="rename_row", classes="rename-row"):
                yield Input(value=self.filename, id="inp_rename", classes="rename-input")
                yield Button("Save", id="btn_save_rename", variant="success", classes="compact-btn")
                yield Button("Cancel", id="btn_cancel_rename", classes="compact-btn")

    def on_mount(self):
        self.query_one("#rename_row").display = False

    async def on_button_pressed(self, event: Button.Pressed) -> None:
        btn_id = event.button.id

        if btn_id == "btn_start_rename":
            self.query_one("#file_row").display = False
            self.query_one("#rename_row").display = True
            inp = self.query_one("#inp_rename", Input)
            inp.value = self.filename
            inp.focus()

        elif btn_id == "btn_cancel_rename":
            self.query_one("#rename_row").display = False
            self.query_one("#file_row").display = True

        elif btn_id == "btn_save_rename":
            new_name = self.query_one("#inp_rename", Input).value.strip()
            if new_name and new_name != self.filename:
                # Need to run ADB mv
                base_dir = self.filepath.rsplit('/', 1)[0]
                new_path = f"{base_dir}/{new_name}"

                if self.is_internal:
                    cmd = f"run-as {PACKAGE} mv {self.filepath} {new_path}"
                    await run_adb_cmd("-s", self.serial, "shell", cmd)
                else:
                    cmd = f"mv {self.filepath} {new_path}"
                    await run_adb_cmd("-s", self.serial, "shell", cmd)

                self.app.notify(f"Renamed {self.filename} to {new_name}")
                self.post_message(self.FileChanged())
            else:
                self.query_one("#rename_row").display = False
                self.query_one("#file_row").display = True

        elif btn_id == "btn_delete":
            event.button.disabled = True
            if self.is_internal:
                cmd = f"run-as {PACKAGE} rm {self.filepath}"
                await run_adb_cmd("-s", self.serial, "shell", cmd)
            else:
                cmd = f"rm {self.filepath}"
                await run_adb_cmd("-s", self.serial, "shell", cmd)

            self.app.notify(f"Deleted {self.filename}")
            self.post_message(self.FileChanged())

        elif btn_id == "btn_download":
            event.button.disabled = True
            event.button.label = "..."
            self.app.notify(f"Downloading {self.filename} to {os.getcwd()}...", title="Download Started")
            self.run_worker(self.download_file(event.button), exclusive=True)

        elif btn_id == "btn_load":
            self.app.notify(f"Loading {self.filename} on device...", title="Load Test")
            self.run_worker(self.load_on_device(), exclusive=True)

    async def load_on_device(self) -> None:
        cmd_args = [
            "-s", self.serial, "shell", "am", "start",
            "-n", f"{PACKAGE}/.MainActivity",
            "--es", "zip_path", self.filename
        ]
        await run_adb_cmd(*cmd_args)
        self.post_message(self.TestLoaded(self.filename))
        self.app.notify(f"Requested device to load {self.filename}", title="Load Complete")

    async def download_file(self, button: Button) -> None:
        dest = os.path.join(os.getcwd(), self.filename)

        if self.is_internal:
            # internal requires run-as
            cmd = f"adb -s {self.serial} shell \"run-as {PACKAGE} cat {self.filepath}\" > \"{dest}\""
            proc = await asyncio.create_subprocess_shell(cmd)
            await proc.communicate()
        else:
            # external can be a direct pull
            await run_adb_cmd("-s", self.serial, "pull", self.filepath, dest)

        button.label = "Downloaded ✓"
        button.variant = "success"
        self.app.notify(f"Saved: {dest}", title="Complete")
        await asyncio.sleep(2)
        button.label = "Download"
        button.disabled = False
        button.variant = "primary"

class MainScreen(Screen):
    device_serial = reactive("")
    is_connected = reactive(True)
    current_test = reactive("")
    is_foreground = reactive(False)

    def __init__(self, serial=None, **kwargs):
        super().__init__(**kwargs)
        self.device_serial = serial or ""
        self.tests_seen = set()
        self.results_seen = set()

    def compose(self) -> ComposeResult:
        yield Header()
        yield Label(id="banner", classes="banner")

        with Vertical(id="launch_container"):
            yield Label("App is not running in the foreground.", id="lbl_launch_msg")
            yield Button("Launch App", id="btn_launch_app", variant="success", classes="main-action-btn launch-btn")

        with Horizontal(id="main_container"):
            with Vertical(classes="column"):
                yield Label("🧪 Tests", classes="col-title")
                yield Button("Upload Local Test Bundle", id="btn_upload_test", variant="primary", classes="main-action-btn")
                yield Button("Generate Goldens", id="btn_gen_goldens", variant="warning", classes="main-action-btn")
                yield Button("Generate Test Bundle", id="btn_gen_test", variant="success", classes="main-action-btn")
                yield ScrollableContainer(id="test_list")
            with Vertical(classes="column"):
                yield Label("📊 Results", classes="col-title")
                yield Label("Current: Default", id="lbl_current_test", classes="col-subtitle")
                yield Button("Run test", id="btn_gen_res", variant="success", classes="main-action-btn")
                yield ScrollableContainer(id="result_list")
        yield Footer()

    def on_file_item_test_loaded(self, event: FileItem.TestLoaded) -> None:
        self.current_test = event.filename

    def watch_current_test(self, new_test: str) -> None:
        from textual.css.query import NoMatches
        try:
            if new_test:
                self.query_one("#lbl_current_test", Label).update(f"Current: {new_test}")
        except NoMatches:
            pass

    def watch_is_foreground(self, foreground: bool) -> None:
        from textual.css.query import NoMatches
        try:
            lc = self.query_one("#launch_container")
            mc = self.query_one("#main_container")
            if foreground:
                lc.display = False
                mc.display = True
            else:
                lc.display = True
                mc.display = False
        except NoMatches:
            pass

    def on_file_item_file_changed(self, event: FileItem.FileChanged) -> None:
        """Called when a child file is deleted or renamed to force a full refresh."""
        self.query_one("#test_list").remove_children()
        self.query_one("#result_list").remove_children()
        self.tests_seen.clear()
        self.results_seen.clear()

    def watch_device_serial(self, serial: str) -> None:
        from textual.css.query import NoMatches
        try:
            self.query_one(Header).title = "Filament Validation Runner"
            self.query_one(Header).sub_title = f"Device: {serial}"
        except NoMatches:
            pass

    def watch_is_connected(self, connected: bool) -> None:
        from textual.css.query import NoMatches
        try:
            banner = self.query_one("#banner", Label)
            if connected:
                banner.update("Status: Device Connected ✅")
                banner.remove_class("disconnected")
            else:
                banner.update("Status: Disconnected - Retrying... ❌")
                banner.add_class("disconnected")
        except NoMatches:
            pass

    def on_mount(self) -> None:
        # Check ADB constantly every 2 seconds
        self.set_interval(2.0, self.poll_adb)

        # Hide conditionally
        self.query_one("#launch_container").display = False
        self.query_one("#main_container").display = False

    async def get_files(self, directory: str, is_internal: bool):
        if is_internal:
            code, out, err = await run_adb_cmd("-s", self.device_serial, "shell", f"run-as {PACKAGE} ls -1 {directory}")
        else:
            code, out, err = await run_adb_cmd("-s", self.device_serial, "shell", f"ls -1 {directory}")

        if code == 0 and "Permission denied" not in err and "No such file or directory" not in err:
            return [f.strip() for f in out.splitlines() if f.strip() and f.strip().endswith(".zip")]
        return []

    async def poll_adb(self) -> None:
        code, out, err = await run_adb_cmd("-s", self.device_serial, "shell", "echo ok")
        if code != 0 or "ok" not in out:
            self.is_connected = False
            self.is_foreground = False
            return

        self.is_connected = True

        # Check foreground
        proc = await asyncio.create_subprocess_shell(
            f"adb -s {self.device_serial} shell \"dumpsys window | grep mCurrentFocus\"",
            stdout=asyncio.subprocess.PIPE,
            stderr=asyncio.subprocess.PIPE
        )
        stdout, _ = await proc.communicate()
        out = stdout.decode('utf-8', errors='ignore')

        self.is_foreground = PACKAGE in out

        # We only really need to pull files if we are in the foreground
        if not self.is_foreground:
            return

        current_tests = set()
        current_results = set()
        files_dict = {}

        # 1. External
        ext_files = await self.get_files(EXTERNAL_DIR, False)
        for f in ext_files:
            files_dict[f] = (f"{EXTERNAL_DIR}/{f}", False)

        # 2. Internal
        int_files = await self.get_files(INTERNAL_DIR, True)
        for f in int_files:
            if f not in files_dict:
                files_dict[f] = (f"{INTERNAL_DIR}/{f}", True)

        for f, (path, is_int) in files_dict.items():
            if f.startswith("results_"):
                current_results.add((f, path, is_int))
            else:
                # Any other .zip file is considered a test
                current_tests.add((f, path, is_int))

        # Check if any files were removed (e.g. wiped data)
        removed_tests = self.tests_seen - current_tests
        removed_results = self.results_seen - current_results

        if removed_tests or removed_results:
            self.query_one("#test_list").remove_children()
            self.query_one("#result_list").remove_children()
            self.tests_seen.clear()
            self.results_seen.clear()

        # Update lists safely if they are new
        new_tests = current_tests - self.tests_seen
        test_list = self.query_one("#test_list", ScrollableContainer)
        for f, path, is_int in new_tests:
            await test_list.mount(FileItem(f, path, is_int, self.device_serial))
            test_list.scroll_end(animate=False)
        self.tests_seen.update(new_tests)

        new_results = current_results - self.results_seen
        result_list = self.query_one("#result_list", ScrollableContainer)
        for f, path, is_int in new_results:
            await result_list.mount(FileItem(f, path, is_int, self.device_serial))
            result_list.scroll_end(animate=False)
        self.results_seen.update(new_results)

    async def on_button_pressed(self, event: Button.Pressed) -> None:
        btn_id = event.button.id
        if btn_id == "btn_gen_goldens":
            event.button.disabled = True
            event.button.label = "Generating Goldens..."
            self.run_worker(self.generate_and_auto_export(event.button, "golden"), exclusive=True)
        elif btn_id == "btn_gen_test":
            event.button.disabled = True
            event.button.label = "Generating & Exporting..."
            self.run_worker(self.generate_and_auto_export(event.button, "test"), exclusive=True)
        elif btn_id == "btn_gen_res":
            event.button.disabled = True
            event.button.label = "Generating & Exporting..."
            self.run_worker(self.generate_and_auto_export(event.button, "result"), exclusive=True)
        elif btn_id == "btn_upload_test":
            self.app.push_screen(FileSelectScreen(), self.handle_upload)
        elif btn_id == "btn_launch_app":
            self.run_worker(self.launch_app(), exclusive=True)

    async def launch_app(self) -> None:
        self.app.notify("Commanding device to start the app...", title="Launching")
        await run_adb_cmd("-s", self.device_serial, "shell", "am", "start", "-n", f"{PACKAGE}/.MainActivity")

    def handle_upload(self, file_path: str | None) -> None:
        if file_path:
            self.run_worker(self.upload_file(file_path), exclusive=True)

    async def upload_file(self, file_path: str):
        filename = os.path.basename(file_path)
        self.app.notify(f"Uploading {filename}...", title="Upload Started")

        # We push to EXTERNAL_DIR because `adb push` works seamlessly there
        dest = f"{EXTERNAL_DIR}/{filename}"
        code, out, err = await run_adb_cmd("-s", self.device_serial, "push", file_path, dest)

        if code == 0:
            self.app.notify(f"Successfully uploaded to device.", title="Upload Finished")
            # Force a refresh
            self.tests_seen.clear()
            self.query_one("#test_list").remove_children()
        else:
            self.app.notify(f"Upload failed: {err}", title="Error", severity="error")

    async def generate_and_auto_export(self, button: Button, mode: str):
        self.app.notify("Commanding device...", title="Working")

        # Notice we removed '-S' so it doesn't force stop the activity first
        cmd_args = [
            "-s", self.device_serial, "shell", "am", "start",
            "-n", f"{PACKAGE}/.MainActivity"
        ]

        if mode == "golden":
            cmd_args.extend(["--ez", "generate_goldens", "true", "--ez", "auto_run", "true"])
        elif mode == "test":
            cmd_args.extend(["--ez", "auto_run", "true", "--ez", "auto_export", "true"])
        elif mode == "result":
            cmd_args.extend(["--ez", "auto_run", "true", "--ez", "auto_export_results", "true"])
            if self.current_test:
                cmd_args.extend(["--es", "zip_path", self.current_test])

        await run_adb_cmd(*cmd_args)

        # Allow time to run and let auto-polling grab the new file (if exporting)
        await asyncio.sleep(6)

        button.disabled = False
        if mode == "golden":
            button.label = "Generate Goldens"
        elif mode == "test":
            button.label = "Generate & Download New Test"
        elif mode == "result":
            button.label = "Generate & Download Results"

        self.app.notify("Action Finished")

class ValidationApp(App):
    CSS = """
    Screen {
        layout: vertical;
    }
    .banner {
        width: 100%;
        content-align: center middle;
        background: $success;
        color: $text;
        padding: 1;
        text-style: bold;
    }
    .banner.disconnected {
        background: $error;
    }
    #launch_container {
        align: center middle;
        height: 1fr;
    }
    #lbl_launch_msg {
        text-align: center;
        margin-bottom: 2;
    }
    .launch-btn {
        width: 30;
    }
    #main_container {
        height: 1fr;
        layout: horizontal;
    }
    .column {
        width: 50%;
        height: 100%;
        border: solid $accent;
        padding: 1;
    }
    .col-title {
        text-align: center;
        text-style: bold;
        width: 100%;
        margin-bottom: 1;
    }
    .col-subtitle {
        text-align: center;
        width: 100%;
        margin-bottom: 1;
        color: $secondary;
        text-style: italic;
    }
    .main-action-btn {
        width: 100%;
        margin-bottom: 1;
    }
    .file-item-container {
        height: auto;
        padding-bottom: 1;
        border-bottom: solid $surface;
    }
    .file-item-row {
        height: 3;
        width: 100%;
        align: left middle;
    }
    .file-name {
        width: 1fr;
        content-align: left middle;
    }
    .compact-btn {
        min-width: 4;
        width: auto;
        margin-left: 1;
    }
    .rename-row {
        height: 3;
        width: 100%;
    }
    .rename-input {
        width: 1fr;
    }
    """
    BINDINGS = [
        ("q", "quit", "Quit application")
    ]

    async def on_mount(self) -> None:
        code, out, err = await run_adb_cmd("devices")
        devices = []
        for line in out.splitlines()[1:]:
            if line.strip() and "device" in line:
                devices.append(line.split()[0])

        if len(devices) > 1:
            self.push_screen(DeviceSelectScreen(), self.start_main)
        elif len(devices) == 1:
            self.start_main(devices[0])
        else:
            self.notify("No devices connected via ADB!", severity="error")
            self.start_main("")

    def start_main(self, serial: str) -> None:
        self.push_screen(MainScreen(serial=serial))

if __name__ == "__main__":
    app = ValidationApp()
    app.run()
