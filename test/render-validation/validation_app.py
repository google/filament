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

class DeviceSelectScreen(ModalScreen[str]):
    CSS = """
    DeviceSelectScreen {
        align: center middle;
    }
    #device_dialog {
        width: 60%;
        max-width: 90;
        height: 60%;
        padding: 1;
        border: thick $background 80%;
        background: $surface;
    }
    .device-btn {
        width: 100%;
        margin-bottom: 1;
    }
    """
    def __init__(self, current_serial: str | None = None, **kwargs):
        super().__init__(**kwargs)
        self.current_serial = current_serial

    def compose(self) -> ComposeResult:
        with Vertical(id="device_dialog"):
            yield Label("Select an Android device:", id="title", classes="col-title")
            yield ScrollableContainer(id="device_list")
            yield Button("Cancel", id="btn_cancel_device", variant="error", classes="device-btn")

    async def on_mount(self) -> None:
        code, out, err = await run_adb_cmd("devices", "-l")
        container = self.query_one("#device_list", ScrollableContainer)
        for line in out.splitlines()[1:]:
            if line.strip() and "device " in line:
                parts = line.split()
                serial = parts[0]
                model = "Unknown Device"
                for part in parts:
                    if part.startswith("model:"):
                        model = part.split(":")[1].replace("_", " ")
                        break

                is_current = (serial == self.current_serial)
                btn_label = f"{model} ({serial})" + (" - Current" if is_current else "")
                variant = "success" if is_current else "primary"

                await container.mount(Button(btn_label, id=f"dev_{serial}", classes="device-btn", variant=variant))

    def on_button_pressed(self, event: Button.Pressed) -> None:
        if event.button.id == "btn_cancel_device":
            self.dismiss(None)
        elif event.button.id and event.button.id.startswith("dev_"):
            serial = event.button.id.replace("dev_", "")
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
                else:
                    yield Button("🌐", id="btn_serve", variant="success", classes="compact-btn", tooltip="Serve and view results locally")

                yield Button("↓", id="btn_download", variant="primary", classes="compact-btn", tooltip="Download to PC")
                yield Button("✎", id="btn_start_rename", variant="warning", classes="compact-btn", tooltip="Rename on device")
                yield Button("✗", id="btn_delete", variant="error", classes="compact-btn", tooltip="Delete on device")
            with Horizontal(id="rename_row", classes="rename-row"):
                yield Input(value=self.filename, id="inp_rename", classes="rename-input")
                yield Button("Save", id="btn_save_rename", variant="success", classes="compact-btn")
                yield Button("Cancel", id="btn_cancel_rename", classes="compact-btn")
            yield Label("", id="lbl_server_url", classes="server-url")

    def on_mount(self):
        self.query_one("#rename_row").display = False
        self.query_one("#lbl_server_url").display = False

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

        elif btn_id == "btn_serve":
            if hasattr(self, "server_proc") and self.server_proc:
                self.stop_server(event.button)
            else:
                event.button.disabled = True
                self.run_worker(self.start_server(event.button), exclusive=True)

    async def start_server(self, button: Button) -> None:
        try:
            # Create a tmp directory for the results
            tmp_dir = os.path.join(os.getcwd(), "tmp")
            os.makedirs(tmp_dir, exist_ok=True)
            dest = os.path.join(tmp_dir, self.filename)

            # Download file to tmp
            if not os.path.exists(dest):
                self.app.notify(f"Downloading {self.filename} for viewer...", title="Preparing Server")
                if self.is_internal:
                    cmd = f"adb -s {self.serial} shell \"run-as {PACKAGE} cat {self.filepath}\" > \"{dest}\""
                    proc = await asyncio.create_subprocess_shell(cmd)
                    await proc.communicate()
                else:
                    await run_adb_cmd("-s", self.serial, "pull", self.filepath, dest)

            # Find unoccupied port
            import socket
            s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            s.bind(("", 0))
            port = s.getsockname()[1]
            s.close()

            # Start server
            server_script = os.path.join(os.path.dirname(os.path.abspath(__file__)), "result-viewer", "server.py")
            self.server_proc = await asyncio.create_subprocess_exec(
                "python3", server_script, dest, "--port", str(port)
            )

            button.label = "🛑"
            button.variant = "error"
            button.tooltip = "Stop Server"

            lbl_url = self.query_one("#lbl_server_url", Label)
            lbl_url.update(f"  ↳ Server: http://localhost:{port}")
            lbl_url.display = True

            self.app.notify(f"Result viewer started on http://localhost:{port}", title="Server Started")
        except Exception as e:
            self.app.notify(f"Failed to start server: {e}", title="Server Error", severity="error")
            button.label = "🌐"
            button.variant = "success"
            button.tooltip = "Serve and view results locally"
            self.query_one("#lbl_server_url", Label).display = False
            self.server_proc = None
        finally:
            button.disabled = False

    def stop_server(self, button: Button = None) -> None:
        if hasattr(self, "server_proc") and self.server_proc:
            try:
                self.server_proc.terminate()
            except ProcessLookupError:
                pass
            self.server_proc = None
            if button:
                button.label = "🌐"
                button.variant = "success"
                button.tooltip = "Serve and view results locally"
            try:
                self.query_one("#lbl_server_url", Label).display = False
            except Exception:
                pass
            self.app.notify("Server stopped", title="Server Stopped")

    def on_unmount(self) -> None:
        self.stop_server()

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
    BINDINGS = [
        ("d", "switch_device", "Switch Device")
    ]

    device_serial = reactive("")
    device_model = reactive("Unknown Device")
    is_connected = reactive(True)
    current_test = reactive("")
    is_foreground = reactive(False)

    def __init__(self, serial=None, **kwargs):
        super().__init__(**kwargs)
        self.device_serial = serial or ""
        self.tests_seen = set()
        self.results_seen = set()

    def action_switch_device(self) -> None:
        self.app.push_screen(DeviceSelectScreen(current_serial=self.device_serial), self.handle_device_switch)

    def handle_device_switch(self, serial: str | None) -> None:
        if serial:
            self.device_serial = serial
            self.is_connected = True
            # Force a refresh of the UI
            self.tests_seen.clear()
            self.results_seen.clear()
            self.query_one("#test_list").remove_children()
            self.query_one("#result_list").remove_children()

    def compose(self) -> ComposeResult:
        yield Header()
        with Vertical(id="main_wrapper"):
            yield Label(id="banner", classes="banner")

            with Vertical(id="launch_container"):
                yield Label("App is not running in the foreground.", id="lbl_launch_msg")
                yield Button("Launch App", id="btn_launch_app", variant="success", classes="main-action-btn launch-btn")

            with Horizontal(id="main_container"):
                with Vertical(classes="column"):
                    yield Label("🧪 Tests", classes="col-title")
                    yield Button("Upload Local Test Bundle", id="btn_upload_test", variant="primary", classes="main-action-btn")
                    yield ScrollableContainer(id="test_list")
                with Vertical(classes="column"):
                    yield Label("📊 Results", classes="col-title")
                    yield Label("Current: None", id="lbl_current_test", classes="col-subtitle")
                    yield Button("Generate Goldens", id="btn_gen_res", variant="success", classes="main-action-btn")
                    yield ScrollableContainer(id="result_list")
        yield Footer()

    def on_file_item_test_loaded(self, event: FileItem.TestLoaded) -> None:
        self.current_test = event.filename

    def watch_current_test(self, new_test: str) -> None:
        from textual.css.query import NoMatches
        try:
            if new_test:
                self.query_one("#lbl_current_test", Label).update(f"Current: {new_test}")
                self.query_one("#btn_gen_res", Button).label = "Run test"
            else:
                self.query_one("#lbl_current_test", Label).update("Current: None")
                self.query_one("#btn_gen_res", Button).label = "Generate Goldens"
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

    async def watch_device_serial(self, serial: str) -> None:
        if serial:
            code, out, err = await run_adb_cmd("devices", "-l")
            model = "Unknown Device"
            for line in out.splitlines()[1:]:
                if line.strip() and "device " in line and serial in line:
                    parts = line.split()
                    for part in parts:
                        if part.startswith("model:"):
                            model = part.split(":")[1].replace("_", " ")
                            break
                    break
            self.device_model = model
        else:
            self.device_model = "None"

    def watch_device_model(self, model: str) -> None:
        self.update_header_and_banner()

    def watch_is_connected(self, connected: bool) -> None:
        self.update_header_and_banner()

    def update_header_and_banner(self) -> None:
        from textual.css.query import NoMatches
        try:
            self.query_one(Header).title = "Filament Validation Runner"
            self.query_one(Header).sub_title = f"Device: {self.device_model} ({self.device_serial})"

            banner = self.query_one("#banner", Label)
            if self.is_connected:
                banner.update(f"Status: Connected to {self.device_model} ({self.device_serial}) ✅")
                banner.remove_class("disconnected")
            else:
                banner.update(f"Status: Disconnected from {self.device_model} ({self.device_serial}) - Retrying... ❌")
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

        if not self.current_test and len(current_tests) == 1:
            only_test = list(current_tests)[0]
            if only_test[0] == "default_test.zip":
                self.current_test = "default_test.zip"
                self.app.notify("Auto-loaded default_test.zip")

    async def on_button_pressed(self, event: Button.Pressed) -> None:
        btn_id = event.button.id
        if btn_id == "btn_gen_res":
            event.button.disabled = True
            if not self.current_test:
                event.button.label = "Generating Goldens..."
                self.run_worker(self.generate_and_auto_export(event.button, "golden"), exclusive=True)
            else:
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
        elif mode == "result":
            cmd_args.extend(["--ez", "auto_run", "true"])

        if self.current_test:
            cmd_args.extend(["--es", "zip_path", self.current_test])

        await run_adb_cmd(*cmd_args)

        # Allow time to run and let auto-polling grab the new file (if exporting)
        await asyncio.sleep(6)

        button.disabled = False
        if mode == "golden":
            button.label = "Run test" if self.current_test else "Generate Goldens"
        elif mode == "result":
            button.label = "Run test"

        self.app.notify("Action Finished")

class ValidationApp(App):
    CSS = """
    Screen {
        layout: vertical;
    }
    MainScreen {
        align: center top;
    }
    #main_wrapper {
        width: 100%;
        max-width: 100;
        height: 1fr;
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
    .server-url {
        width: 100%;
        color: $success;
        text-style: italic;
        padding-left: 2;
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

    def start_main(self, serial: str | None) -> None:
        self.push_screen(MainScreen(serial=serial or ""))

if __name__ == "__main__":
    app = ValidationApp()
    app.run()
