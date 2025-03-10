///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// EditorForm.cs                                                             //
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
// This file is distributed under the University of Illinois Open Source     //
// License. See LICENSE.TXT for details.                                     //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

using DotNetDxc;
using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Diagnostics;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Text.RegularExpressions;
using System.Windows.Forms;

namespace MainNs
{
    public partial class EditorForm : Form
    {
        private const string AppName = "DirectX Compiler Editor";
        private const string OptDescSeparator = "\t";
        private bool autoDisassemble;
        private IDxcLibrary library;
        private IDxcIntelliSense isense;
        private IDxcIndex lastIndex;
        private IDxcTranslationUnit lastTU;
        private IDxcBlob selectedShaderBlob;
        private string selectedShaderBlobTarget;
        private bool passesLoaded = false;
        private bool docModified = false;
        private string docFileName;
        private DocumentKind documentKind;
        private MRUManager mruManager;
        private SettingsManager settingsManager;
        private Action pendingASTDump;
        private FindDialog findDialog;
        private TabPage errorListTabPage;
        private List<DiagnosticDetail> diagnosticDetails;
        private DataGridView diagnosticDetailsGrid;
        private List<PassInfo> passInfos;
        private TabPage debugInfoTabPage;
        private RichTextBox debugInfoControl;
        private HlslHost hlslHost = new HlslHost();
        private TabPage renderViewTabPage;
        private TabPage rewriterOutputTabPage;
        private TabPage helpTabPage;
        private RichTextBox helpControl;

        internal enum DocumentKind
        {
            /// <summary>
            /// HLSL source code.
            /// </summary>
            HlslText,
            /// <summary>
            /// LLVM source code.
            /// </summary>
            AsmText,
            /// <summary>
            /// Compiled DXIL container.
            /// </summary>
            CompiledObject,
        }

        public EditorForm()
        {
            InitializeComponent();
            cbProfile.SelectedIndex = 0;
        }

        internal IDxcBlob SelectedShaderBlob
        {
            get { return this.selectedShaderBlob; }
            set
            {
                this.selectedShaderBlob = value;
                this.selectedShaderBlobTarget = TryGetBlobShaderTarget(this.selectedShaderBlob);
            }
        }

        private const uint DFCC_DXIL = 1279875140;
        private const uint DFCC_SHDR = 1380206675;
        private const uint DFCC_SHEX = 1480935507;
        private const uint DFCC_ILDB = 1111772233;
        private const uint DFCC_SPDB = 1111773267;

        private TabPage HelpTabPage
        {
            get
            {
                if (this.helpTabPage == null)
                {
                    this.helpTabPage = new TabPage("Help");
                    this.AnalysisTabControl.TabPages.Add(helpTabPage);
                }
                return this.helpTabPage;
            }
        }

        private RichTextBox HelpControl
        {
            get
            {
                if (this.helpControl == null)
                {
                    this.helpControl = new RichTextBox();
                    this.HelpTabPage.Controls.Add(this.helpControl);
                    this.helpControl.Dock = DockStyle.Fill;
                    this.helpControl.Font = this.CodeBox.Font;
                }
                return this.helpControl;
            }
        }

        private TabPage RenderViewTabPage
        {
            get
            {
                if (this.renderViewTabPage == null)
                {
                    this.renderViewTabPage = new TabPage("Render View");
                    this.AnalysisTabControl.TabPages.Add(renderViewTabPage);
                }
                return this.renderViewTabPage;
            }
        }

        private TabPage RewriterOutputTabPage
        {
            get
            {
                if (this.rewriterOutputTabPage == null)
                {
                    this.rewriterOutputTabPage = new TabPage("Rewriter Output");
                    this.AnalysisTabControl.TabPages.Add(rewriterOutputTabPage);
                }
                return this.rewriterOutputTabPage;
            }
        }

        private string TryGetBlobShaderTarget(IDxcBlob blob)
        {
            if (blob == null)
                return null;
            uint size = blob.GetBufferSize();
            if (size == 0) return null;
            unsafe
            {
                try
                {
                    var reflection = HlslDxcLib.CreateDxcContainerReflection();
                    reflection.Load(blob);
                    uint index;
                    int hr = reflection.FindFirstPartKind(DFCC_DXIL, out index);
                    if (hr < 0)
                        hr = reflection.FindFirstPartKind(DFCC_SHEX, out index);
                    if (hr < 0)
                        hr = reflection.FindFirstPartKind(DFCC_SHDR, out index);
                    if (hr < 0)
                        return null;
                    unsafe
                    {
                        IDxcBlob part = reflection.GetPartContent(index);
                        UInt32* p = (UInt32*)part.GetBufferPointer();
                        return DescribeProgramVersionShort(*p);
                    }
                }
                catch (Exception)
                {
                    return null;
                }
            }
        }

        private void EditorForm_Shown(object sender, EventArgs e)
        {
            // Launched as a console program, so this needs to be done explicitly.
            this.Activate();
            this.UpdateWindowText();
        }

        #region Menu item handlers.

        private void aboutToolStripMenuItem_Click(object sender, EventArgs e)
        {
            // Version information currently is available through validator.
            string libraryVersion;
            try
            {
                IDxcValidator validator = HlslDxcLib.CreateDxcValidator();
                IDxcVersionInfo versionInfo = (IDxcVersionInfo)validator;
                uint major, minor;
                versionInfo.GetVersion(out major, out minor);
                DxcVersionInfoFlags flags = versionInfo.GetFlags();
                libraryVersion = major.ToString() + "." + minor.ToString();
                if ((flags & DxcVersionInfoFlags.Debug) == DxcVersionInfoFlags.Debug)
                {
                    libraryVersion += " (debug)";
                }
            }
            catch (Exception err)
            {
                libraryVersion = err.Message;
            }
            IDxcLibrary library = HlslDxcLib.CreateDxcLibrary();
            MessageBox.Show(this,
                AppName + "\r\n" +
                "Compiler Library Version: " + libraryVersion + "\r\n" +
                "See LICENSE.txt for license information.", "About " + AppName);
        }

        private void compileToolStripMenuItem_Click(object sender, EventArgs e)
        {
            this.CompileDocument();
            this.AnalysisTabControl.SelectedTab = this.DisassemblyTabPage;
        }

        private void errorListToolStripMenuItem_Click(object sender, EventArgs e)
        {
            if (errorListTabPage == null)
            {
                this.errorListTabPage = new TabPage("Error List");
                this.AnalysisTabControl.TabPages.Add(this.errorListTabPage);
                this.diagnosticDetailsGrid = new DataGridView()
                {
                    AutoSizeColumnsMode = DataGridViewAutoSizeColumnsMode.AllCells,
                    Dock = DockStyle.Fill,
                    ReadOnly = true,
                };
                this.diagnosticDetailsGrid.DoubleClick += DiagnosticDetailsGridDoubleClick;
                this.diagnosticDetailsGrid.CellDoubleClick += (_, __) => { DiagnosticDetailsGridDoubleClick(sender, EventArgs.Empty); };
                this.errorListTabPage.Controls.Add(this.diagnosticDetailsGrid);
                this.RefreshDiagnosticDetails();
            }
            this.AnalysisTabControl.SelectedTab = this.errorListTabPage;
        }

        private void exitToolStripMenuItem_Click(object sender, EventArgs e)
        {
            this.Close();
        }

        private void fileVariablesToolStripMenuItem_Click(object sender, EventArgs e)
        {
            string codeText = this.CodeBox.Text;
            HlslFileVariables fileVars = GetFileVars();
            using (Form form = new Form())
            {
                PropertyGrid grid = new PropertyGrid()
                {
                    Dock = DockStyle.Fill,
                    SelectedObject = fileVars,
                };
                Button okButton = new Button()
                {
                    Dock = DockStyle.Bottom,
                    DialogResult = DialogResult.OK,
                    Text = "OK"
                };
                Button cancelButton = new Button()
                {
                    DialogResult = DialogResult.Cancel,
                    Text = "Cancel"
                };
                form.Controls.Add(grid);
                form.Controls.Add(okButton);
                if (form.ShowDialog(this) == DialogResult.OK)
                {
                    string newFirstLine = fileVars.ToString();
                    if (fileVars.SetFromText)
                    {
                        int firstEnd = codeText.IndexOf('\n');
                        if (firstEnd == 0)
                        {
                            codeText = "// " + fileVars.ToString();
                        }
                        else
                        {
                            codeText = "// " + fileVars.ToString() + "\r\n" + codeText.Substring(firstEnd + 1);
                        }
                    }
                    else
                    {
                        codeText = "// " + fileVars.ToString() + "\r\n" + codeText;
                    }
                    this.CodeBox.Text = codeText;
                }
            }
        }

        private void NewToolStripMenuItem_Click(object sender, EventArgs e)
        {
            if (!IsOKToOverwriteContent())
                return;

            // Consider: using this a simpler File | New experience.
            //string psBanner =
            //    "// -*- mode: hlsl; hlsl-entry: main; hlsl-target: ps_6_0; hlsl-args: /Zi; -*-\r\n";
            //string simpleShader =
            //    "float4 main() : SV_Target {\r\n  return 1;\r\n}\r\n";

            string shaderSample =
                "// -*- mode: hlsl; hlsl-entry: VSMain; hlsl-target: vs_6_0; hlsl-args: /Zi; -*-\r\n" +
               "struct PSInput {\r\n" +
                " float4 position : SV_POSITION;\r\n" +
                " float4 color : COLOR;\r\n" +
                "};\r\n" +
                "[RootSignature(\"RootFlags(ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT)\")]\r\n" +
                "PSInput VSMain(float4 position: POSITION, float4 color: COLOR) {\r\n" +
                " float aspect = 320.0 / 200.0;\r\n" +
                " PSInput result;\r\n" +
                " result.position = position;\r\n" +
                " result.position.y *= aspect;\r\n" +
                " result.color = color;\r\n" +
                " return result;\r\n" +
                "}\r\n" +
                "[RootSignature(\"RootFlags(ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT)\")]\r\n" +
               "float4 PSMain(PSInput input) : SV_TARGET {\r\n" +
               " return input.color;\r\n" +
               "}\r\n";

            string xmlSample =
                "<ShaderOp PS='PS' VS='VS'>\r\n" +
                " <RootSignature>RootFlags(ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT)</RootSignature>\r\n" +
                " <Resource Name='VBuffer' Dimension='BUFFER' Width='1024' Flags='ALLOW_UNORDERED_ACCESS' InitialResourceState='COPY_DEST' Init='FromBytes'>\r\n" +
                "  { {   0.0f,  0.25f , 0.0f }, { 1.0f, 0.0f, 0.0f, 1.0f } },\r\n" +
                "  { {  0.25f, -0.25f , 0.0f }, { 0.0f, 1.0f, 0.0f, 1.0f } },\r\n" +
                "  { { -0.25f, -0.25f , 0.0f }, { 0.0f, 0.0f, 1.0f, 1.0f } }\r\n" +
                " </Resource>\r\n" +
                " <DescriptorHeap Name='RtvHeap' NumDescriptors='1' Type='RTV'>\r\n" +
                " </DescriptorHeap>\r\n" +
                " <InputElements>\r\n" +
                "  <InputElement SemanticName='POSITION' Format='R32G32B32_FLOAT' AlignedByteOffset='0' />\r\n" +
                "  <InputElement SemanticName='COLOR' Format='R32G32B32A32_FLOAT' AlignedByteOffset='12' />\r\n" +
                " </InputElements>\r\n" +
                " <Shader Name='VS' Target='vs_6_0' EntryPoint='VSMain' />\r\n" +
                " <Shader Name='PS' Target='ps_6_0' EntryPoint='PSMain' />\r\n" +
                "</ShaderOp>\r\n";

            this.CodeBox.Text =
                shaderSample + "\r\n" +
                ShaderOpStartMarker + "\r\n" +
                xmlSample +
                ShaderOpStopMarker + "\r\n";

            this.CodeBox.ClearUndo();
            this.DocKind = DocumentKind.HlslText;
            this.DocFileName = null;
            this.DocModified = false;
        }

        private void saveToolStripMenuItem_Click(object sender, EventArgs e)
        {
            if (this.DocKind == DocumentKind.CompiledObject)
            {
                throw new NotImplementedException();
            }

            if (String.IsNullOrEmpty(this.DocFileName))
            {
                saveAsToolStripMenuItem_Click(sender, e);
                return;
            }
            try
            {
                System.IO.File.WriteAllText(this.DocFileName, GetCodeWithFileVars());
            }
            catch (System.IO.IOException)
            {
                this.mruManager.HandleFileFail(this.DocFileName);
                throw;
            }
            this.DocModified = false;

            this.mruManager.HandleFileSave(this.DocFileName);
            this.mruManager.SaveToFile();
        }

        private void saveAsToolStripMenuItem_Click(object sender, EventArgs e)
        {
            using (SaveFileDialog dialog = new SaveFileDialog())
            {
                dialog.DefaultExt = ".hlsl";
                if (!String.IsNullOrEmpty(this.DocFileName))
                    dialog.FileName = this.DocFileName;
                dialog.Filter = "HLSL Files (*.hlsl)|*.hlsl|DXIL Files (*.ll)|*.ll|All Files (*.*)|*.*";
                dialog.ValidateNames = true;
                if (dialog.ShowDialog(this) != DialogResult.OK)
                    return;
                this.DocFileName = dialog.FileName;
            }
            System.IO.File.WriteAllText(this.DocFileName, GetCodeWithFileVars());
            this.DocModified = false;
        }

        private void HandleOpenUI(string mruPath)
        {
            if (!IsOKToOverwriteContent())
                return;

            if (mruPath == null)
            {
                // If not MRU, prompt for a path.
                using (OpenFileDialog dialog = new OpenFileDialog())
                {
                    dialog.DefaultExt = ".hlsl";
                    if (!String.IsNullOrEmpty(this.DocFileName))
                        dialog.FileName = this.DocFileName;
                    dialog.Filter = "HLSL Files (*.hlsl)|*.hlsl|Compiled Shader Objects (*.cso;*.fxc)|*.cso;*.fxc|All Files (*.*)|*.*";
                    dialog.ValidateNames = true;
                    if (dialog.ShowDialog(this) != DialogResult.OK)
                        return;
                    this.DocFileName = dialog.FileName;
                }
            }
            else
            {
                this.DocFileName = mruPath;
            }

            string ext = System.IO.Path.GetExtension(this.DocFileName).ToLowerInvariant();
            if (ext == ".cso" || ext == ".fxc")
            {
                this.SelectedShaderBlob = this.Library.CreateBlobFromFile(this.DocFileName, IntPtr.Zero);
                this.DocKind = DocumentKind.CompiledObject;
                this.DisassembleSelectedShaderBlob();
            }
            else
            {
                this.DocKind = (ext == ".ll") ? DocumentKind.AsmText : DocumentKind.HlslText;
                try
                {
                    this.CodeBox.Text = System.IO.File.ReadAllText(this.DocFileName);
                }
                catch (System.IO.IOException)
                {
                    this.mruManager.HandleFileFail(this.DocFileName);
                    throw;
                }
            }

            this.DocModified = false;
            this.mruManager.HandleFileLoad(this.DocFileName);
            this.mruManager.SaveToFile();
        }

        private void openToolStripMenuItem_Click(object sender, EventArgs e)
        {
            this.HandleOpenUI(null);
        }

        private void exportCompiledObjectToolStripMenuItem_Click(object sender, EventArgs e)
        {
            if (this.SelectedShaderBlob == null)
            {
                MessageBox.Show(this, "There is no compiled shader blob available for exporting.");
                return;
            }

            using (SaveFileDialog dialog = new SaveFileDialog())
            {
                dialog.DefaultExt = ".cso";
                if (!String.IsNullOrEmpty(this.DocFileName))
                {
                    dialog.FileName = this.DocFileName;
                    if (String.IsNullOrEmpty(System.IO.Path.GetExtension(this.DocFileName)))
                    {
                        dialog.FileName += ".cso";
                    }
                }
                dialog.Filter = "Compiled Shader Object Files (*.cso;*.fxc)|*.cso;*.fxc|All Files (*.*)|*.*";
                dialog.ValidateNames = true;
                if (dialog.ShowDialog(this) != DialogResult.OK)
                    return;

                System.IO.File.WriteAllBytes(dialog.FileName, GetBytesFromBlob(this.SelectedShaderBlob));
            }
        }

        private void quickFindToolStripMenuItem_Click(object sender, EventArgs e)
        {
            TextBoxBase target = (this.DeepActiveControl as TextBoxBase);
            if (target == null) return;

            if (findDialog == null)
            {
                this.findDialog = new FindDialog();
                this.findDialog.Disposed += (_sender, _e) =>
                {
                    this.findDialog = null;
                };
            }

            if (target.SelectionLength < 128)
                this.findDialog.FindText = target.SelectedText;
            else
                this.findDialog.FindText = "";
            this.findDialog.Target = target;
            if (this.findDialog.Visible)
            {
                this.findDialog.Focus();
            }
            else
            {
                this.findDialog.Show(this);
            }
        }

        #endregion Menu item handlers.

        private static bool IsDxilTarget(string target)
        {
            // ps_6_0
            // 012345
            int major;
            if (target != null && target.Length == 6)
                if (Int32.TryParse(new string(target[3], 1), out major))
                    return major >= 6;
            if (target.StartsWith("lib"))
                if (Int32.TryParse(new string(target[4], 1), out major))
                    return major >= 6;
            return false;
        }

        private void CompileDocument()
        {
            this.DisassemblyTextBox.Font = this.CodeBox.Font;
            this.ASTDumpBox.Font = this.CodeBox.Font;
            SelectionHighlightData.ClearAnyFromRtb(this.CodeBox);

            var library = this.Library;

            // Switch modes. Can probably be better.
            DocumentKind localKind = this.DocKind;
            string text = null;
            if (localKind == DocumentKind.HlslText || this.DocKind == DocumentKind.AsmText)
            {
                text = this.CodeBox.Text;
                if (String.IsNullOrEmpty(text))
                {
                    return;
                }

                // Make some obvious changes.
                if (text[0] == ';')
                {
                    localKind = DocumentKind.AsmText;
                }
                else if (text[0] == '/')
                {
                    localKind = DocumentKind.HlslText;
                }
            }

            if (localKind == DocumentKind.HlslText)
            {
                var source = this.CreateBlobForText(text);

                string fileName = "hlsl.hlsl";
                HlslFileVariables fileVars = GetFileVars();
                bool isDxil = IsDxilTarget(fileVars.Target);
                IDxcCompiler compiler = isDxil ? HlslDxcLib.CreateDxcCompiler() : null;
                {
                    string[] arguments = fileVars.Arguments;
                    if (isDxil)
                    {
                        try
                        {
                            var result = compiler.Compile(source, fileName, fileVars.Entry, fileVars.Target, arguments, arguments.Length, null, 0, library.CreateIncludeHandler());
                            if (result.GetStatus() == 0)
                            {
                                this.SelectedShaderBlob = result.GetResult();
                                this.DisassembleSelectedShaderBlob();
                            }
                            else
                            {
                                this.colorizationService.ClearColorization(this.DisassemblyTextBox);
                                this.DisassemblyTextBox.Text = GetStringFromBlob(result.GetErrors());
                            }
                        } catch (Exception e)
                        {
                            DisassemblyTextBox.Text = e.ToString();
                        }
                    }
                    else
                    {
                        this.colorizationService.ClearColorization(this.DisassemblyTextBox);
                        IDxcBlob code, errorMsgs;
                        uint flags = 0;
                        const uint D3DCOMPILE_DEBUG = 1;
                        if (fileVars.Arguments.Contains("/Zi")) flags = D3DCOMPILE_DEBUG;
                        int hr = D3DCompiler.D3DCompiler.D3DCompile(text, text.Length, fileName, null, 0, fileVars.Entry, fileVars.Target, flags, 0, out code, out errorMsgs);
                        if (hr != 0)
                        {
                            if (errorMsgs != null)
                            {
                                this.DisassemblyTextBox.Text = GetStringFromBlob(errorMsgs);
                            }
                            else
                            {
                                this.DisassemblyTextBox.Text = "Compilation filed with 0x" + hr.ToString("x");
                            }
                            return;
                        }
                        IDxcBlob disassembly;
                        unsafe
                        {
                            IntPtr buf = new IntPtr(code.GetBufferPointer());
                            hr = D3DCompiler.D3DCompiler.D3DDisassemble(buf, code.GetBufferSize(),
                                0, null, out disassembly);
                            this.DisassemblyTextBox.Text = GetStringFromBlob(disassembly);
                        }
                        this.SelectedShaderBlob = code;
                    }
                }

                // AST Dump - defer to avoid another parse pass
                pendingASTDump = () =>
                {
                    try
                    {
                        List<string> args = new List<string>();
                        args.Add("-ast-dump");
                        args.AddRange(tbOptions.Text.Split());
                        var result = compiler.Compile(source, fileName, fileVars.Entry,
                            fileVars.Target, args.ToArray(), args.Count,
                            null, 0, library.CreateIncludeHandler());
                        if (result.GetStatus() == 0)
                        {
                            this.ASTDumpBox.Text = GetStringFromBlob(result.GetResult());
                        }
                        else
                        {
                            this.ASTDumpBox.Text = GetStringFromBlob(result.GetErrors());
                        }
                    }
                    catch (Exception e)
                    {
                        this.ASTDumpBox.Text = e.ToString();
                    }
                };

                if (AnalysisTabControl.SelectedTab == ASTTabPage)
                {
                    pendingASTDump();
                    pendingASTDump = null;
                }

                if (this.diagnosticDetailsGrid != null)
                {
                    this.RefreshDiagnosticDetails();
                }
            }
            else if (localKind == DocumentKind.CompiledObject)
            {
                this.CodeBox.Text = "Cannot compile a shader object.";
                return;
            }
            else if (localKind == DocumentKind.AsmText)
            {
                var source = this.CreateBlobForText(text);
                var assembler = HlslDxcLib.CreateDxcAssembler();
                var result = assembler.AssembleToContainer(source);
                if (result.GetStatus() == 0)
                {
                    this.SelectedShaderBlob = result.GetResult();
                    this.DisassembleSelectedShaderBlob();
                    // TODO: run validation on this shader blob
                }
                else
                {
                    this.DisassemblyTextBox.Text = GetStringFromBlob(result.GetErrors());
                }

                return;
            }
        }

        class RtbColorization
        {
            private RichTextBox rtb;
            private NumericRanges colored;
            private AsmColorizer colorizer;
            private string text;

            public RtbColorization(RichTextBox rtb, string text)
            {
                this.rtb = rtb;
                this.colorizer = new AsmColorizer();
                this.text = text;
                this.colored = new NumericRanges();
            }

            public void Start()
            {
                this.rtb.SizeChanged += Rtb_SizeChanged;
                this.rtb.HScroll += Rtb_SizeChanged;
                this.rtb.VScroll += Rtb_SizeChanged;
                this.ColorizeVisibleRegion();
            }

            public void Stop()
            {
                this.rtb.SizeChanged -= Rtb_SizeChanged;
                this.rtb.HScroll -= Rtb_SizeChanged;
                this.rtb.VScroll -= Rtb_SizeChanged;
            }

            private void Rtb_SizeChanged(object sender, EventArgs e)
            {
                this.ColorizeVisibleRegion();
            }

            private void ColorizeVisibleRegion()
            {
                int firstCharIdx = rtb.GetCharIndexFromPosition(new Point(0, 0));
                firstCharIdx = rtb.GetFirstCharIndexFromLine(rtb.GetLineFromCharIndex(firstCharIdx));
                int lastCharIdx = rtb.GetCharIndexFromPosition(new Point(rtb.ClientSize));
                NumericRange visibleRange = new NumericRange(firstCharIdx, lastCharIdx + 1);

                if (this.colored.Contains(visibleRange))
                {
                    return;
                }

                var doc = GetTextDocument(rtb);
                using (new RichTextBoxEditAction(rtb))
                {
                    foreach (var idxRange in this.colored.ListGaps(visibleRange))
                    {
                        this.colored.Add(idxRange);
                        foreach (var range in this.colorizer.GetColorRanges(this.text, idxRange.Lo, idxRange.Hi - 1))
                        {
                            if (range.RangeKind == AsmRangeKind.WS) continue;
                            if (range.Start + range.Length < firstCharIdx) continue;
                            if (lastCharIdx < range.Start) return;
                            Color color;
                            switch (range.RangeKind)
                            {
                                case AsmRangeKind.Comment:
                                    color = Color.DarkGreen;
                                    break;
                                case AsmRangeKind.LLVMTypeName:
                                case AsmRangeKind.Keyword:
                                case AsmRangeKind.Instruction:
                                    color = Color.Blue;
                                    break;
                                case AsmRangeKind.StringConstant:
                                    color = Color.DarkRed;
                                    break;
                                case AsmRangeKind.Metadata:
                                    color = Color.DarkOrange;
                                    break;
                                default:
                                    color = Color.Black;
                                    break;
                            }
                            SetStartLengthColor(doc, range.Start, range.Length, color);
                        }
                    }
                }
            }
        }

        class RtbColorizationService
        {
            private Dictionary<RichTextBox, RtbColorization> instances = new Dictionary<RichTextBox, RtbColorization>();

            public void ClearColorization(RichTextBox rtb)
            {
                SetColorization(rtb, null);
            }

            public void SetColorization(RichTextBox rtb, RtbColorization colorization)
            {
                RtbColorization existing;
                if (instances.TryGetValue(rtb, out existing) && existing != null)
                {
                    existing.Stop();
                }

                instances[rtb] = colorization;
                if (colorization != null)
                    colorization.Start();
            }
        }

        RtbColorizationService colorizationService = new RtbColorizationService();

        private void DisassembleSelectedShaderBlob()
        {
            this.DisassemblyTextBox.Font = this.CodeBox.Font;

            var compiler = HlslDxcLib.CreateDxcCompiler();
            try
            {
                var dis = compiler.Disassemble(this.SelectedShaderBlob);
                string disassemblyText = GetStringFromBlob(dis);

                RichTextBox rtb = this.DisassemblyTextBox;
                this.DisassemblyTextBox.Text = disassemblyText;
                this.colorizationService.SetColorization(rtb, new RtbColorization(rtb, disassemblyText));
            }
            catch (Exception e)
            {
                this.DisassemblyTextBox.Text = "Unable to disassemble selected shader.\r\n" + e.ToString();
            }
        }

        private void HandleException(Exception exception)
        {
            HandleException(exception, "Exception " + exception.GetType().Name);
        }

        private void HandleException(Exception exception, string caption)
        {
            MessageBox.Show(this, exception.ToString(), caption);
        }

        private static Dictionary<string, string> ParseFirstLineOptions(string line)
        {
            Dictionary<string, string> result = new Dictionary<string, string>();
            int start = line.IndexOf("-*-");
            if (start < 0) return result;
            int end = line.IndexOf("-*-", start + 3);
            if (end < 0) return result;
            string[] nameValuePairs = line.Substring(start + 3, (end - start - 3)).Split(';');
            foreach (string pair in nameValuePairs)
            {
                int separator = pair.IndexOf(':');
                if (separator < 0) continue;
                string name = pair.Substring(0, separator).Trim();
                string value = pair.Substring(separator + 1).Trim();
                result[name] = value;
            }
            return result;
        }

        class HlslFileVariables
        {
            [Description("Editing mode for the file, typically hlsl")]
            public string Mode { get; set; }
            [Description("Name of the entry point function")]
            public string Entry { get; set; }
            [Description("Shader model target")]
            public string Target { get; set; }
            [Description("Arguments for compilation")]
            public string[] Arguments { get; set; }
            [Description("Whether the variables where obtained from the text file")]
            public bool SetFromText { get; private set; }

            public static HlslFileVariables FromText(string text)
            {
                HlslFileVariables result = new HlslFileVariables();
                int lineEnd = text.IndexOf('\n');
                if (lineEnd > 0) text = text.Substring(0, lineEnd);
                Dictionary<string, string> options = ParseFirstLineOptions(text);
                result.SetFromText = options.Count > 0;
                result.Mode = GetValueOrDefault(options, "mode", "hlsl");
                result.Entry = GetValueOrDefault(options, "hlsl-entry", "main");
                result.Target = GetValueOrDefault(options, "hlsl-target", "ps_6_0");
                result.Arguments = GetValueOrDefault(options, "hlsl-args", "").Split(' ').Select(a => a.Trim()).ToArray();
                return result;
            }

            public override string ToString()
            {
                StringBuilder sb = new StringBuilder();
                sb.Append("-*- ");
                sb.AppendFormat("mode: {0}; ", this.Mode);
                sb.AppendFormat("hlsl-entry: {0}; ", this.Entry);
                sb.AppendFormat("hlsl-target: {0}; ", this.Target);
                if (this.Arguments.Length > 0)
                {
                    sb.AppendFormat("hlsl-args: {0}; ", String.Join(" ", this.Arguments));
                }
                sb.Append("-*-");
                return sb.ToString();
            }
        }

        internal bool AutoDisassemble
        {
            get { return this.autoDisassemble; }
            set
            {
                this.autoDisassemble = value;
                this.autoUpdateToolStripMenuItem.Checked = value;
                if (value)
                {
                    this.CompileDocument();
                }
            }
        }

        private void UpdateWindowText()
        {
            string text = "";
            if (this.DocModified) text = "* ";
            if (!String.IsNullOrEmpty(this.DocFileName))
            {
                text += System.IO.Path.GetFileName(this.DocFileName);
            }
            else
            {
                text += "Untitled";
            }
            text += " - " + AppName;
            this.Text = text;
        }

        internal DocumentKind DocKind
        {
            get { return this.documentKind; }
            set
            {
                this.documentKind = value;
                switch (value)
                {
                    case DocumentKind.AsmText:
                        this.CodeBox.Enabled = true;
                        break;
                    case DocumentKind.HlslText:
                        this.CodeBox.Enabled = true;
                        break;
                    case DocumentKind.CompiledObject:
                        this.CodeBox.Enabled = false;
                        break;
                }
            }
        }

        internal string DocFileName
        {
            get { return this.docFileName; }
            set
            {
                this.docFileName = value;
                this.UpdateWindowText();
            }
        }

        internal bool DocModified
        {
            get { return this.docModified; }
            set
            {
                if (this.docModified != value)
                {
                    this.docModified = value;
                    this.UpdateWindowText();
                }
            }
        }

        internal IDxcLibrary Library
        {
            get { return (library ?? (library = HlslDxcLib.CreateDxcLibrary())); }
        }

        internal bool ShowCodeColor
        {
            get { return ColorMenuItem.Checked; }
            set { ColorMenuItem.Checked = value; }
        }

        internal bool ShowReferences
        {
            get { return ColorMenuItem.Checked; }
            set { ColorMenuItem.Checked = value; }
        }

        internal IDxcTranslationUnit GetTU()
        {
            if (this.lastTU == null)
            {
                if (this.isense == null)
                {
                    this.isense = HlslDxcLib.CreateDxcIntelliSense();
                }
                this.lastIndex = this.isense.CreateIndex();
                IDxcUnsavedFile[] unsavedFiles = new IDxcUnsavedFile[]
                {
                    new TrivialDxcUnsavedFile("hlsl.hlsl", this.CodeBox.Text)
                };
                HlslFileVariables fileVars = GetFileVars();
                this.lastTU = this.lastIndex.ParseTranslationUnit("hlsl.hlsl", fileVars.Arguments, fileVars.Arguments.Length,
                    unsavedFiles, (uint)unsavedFiles.Length, (uint)DxcTranslationUnitFlags.DxcTranslationUnitFlags_UseCallerThread);
            }
            return this.lastTU;
        }

        private HlslFileVariables GetFileVars()
        {
            HlslFileVariables fileVars = HlslFileVariables.FromText(this.CodeBox.Text);
            if (fileVars.SetFromText)
            {
                tbEntry.Text = fileVars.Entry;
                cbProfile.Text = fileVars.Target;
                tbOptions.Text = string.Join(" ", fileVars.Arguments);
            }
            else
            {
                fileVars.Arguments = tbOptions.Text.Split();
                fileVars.Entry = tbEntry.Text;
                fileVars.Target = cbProfile.Text;
            }
            return fileVars;
        }

        private string GetCodeWithFileVars()
        {
            HlslFileVariables fileVars = GetFileVars();
            string codeText = CodeBox.Text;
            if (fileVars.SetFromText)
            {
                int firstEnd = codeText.IndexOf('\n');
                if (firstEnd == 0)
                {
                    codeText = "// " + fileVars.ToString();
                }
                else
                {
                    codeText = "// " + fileVars.ToString() + "\r\n" + codeText.Substring(firstEnd + 1);
                }
            }
            else
            {
                codeText = "// " + fileVars.ToString() + "\r\n" + codeText;
            }
            return codeText;
        }

        internal void InvalidateTU()
        {
            this.lastTU = null;
            this.lastIndex = null;
            this.pendingASTDump = null;
        }

        private IDxcBlobEncoding CreateBlobForCodeText()
        {
            return CreateBlobForText(this.CodeBox.Text);
        }

        private IDxcBlobEncoding CreateBlobForText(string text)
        {
            return CreateBlobForText(this.Library, text);
        }

        public static IDxcBlobEncoding CreateBlobForText(IDxcLibrary library, string text)
        {
            if (String.IsNullOrEmpty(text))
            {
                return null;
            }
            const UInt32 CP_UTF16 = 1200;
            var source = library.CreateBlobWithEncodingOnHeapCopy(text, (UInt32)(text.Length * 2), CP_UTF16);
            return source;
        }

        private void CodeBox_SelectionChanged(object sender, System.EventArgs e)
        {
            if (!this.ShowReferences && !this.ShowCodeColor)
            {
                return;
            }

            IDxcTranslationUnit tu = this.GetTU();
            if (tu == null)
            {
                return;
            }

            RichTextBox rtb = this.CodeBox;
            SelectionHighlightData data = SelectionHighlightData.FromRtb(rtb);
            int start = this.CodeBox.SelectionStart;

            if (this.ShowReferences && rtb.SelectionLength > 0)
            {
                return;
            }


            var doc = GetTextDocument(rtb);
            var mainFile = tu.GetFile(tu.GetFileName());

            using (new RichTextBoxEditAction(rtb))
            {
                data.ClearFromRtb(rtb);
                if (this.ShowCodeColor)
                {
                    // Basic tokenization.
                    IDxcToken[] tokens;
                    uint tokenCount;
                    IDxcSourceRange range = this.isense.GetRange(
                        tu.GetLocationForOffset(mainFile, 0),
                        tu.GetLocationForOffset(mainFile, (uint)this.CodeBox.TextLength));
                    tu.Tokenize(range, out tokens, out tokenCount);
                    if (tokens != null)
                    {
                        foreach (var t in tokens)
                        {
                            switch (t.GetKind())
                            {
                                case DxcTokenKind.Keyword:
                                    uint line, col, offset, endOffset;
                                    IDxcFile file;
                                    t.GetLocation().GetSpellingLocation(out file, out line, out col, out offset);
                                    t.GetExtent().GetEnd().GetSpellingLocation(out file, out line, out col, out endOffset);
                                    SetStartLengthColor(doc, (int)offset, (int)(endOffset - offset), Color.Blue);
                                    break;
                            }
                        }
                    }
                }

                if (this.ShowReferences)
                {
                    var loc = tu.GetLocationForOffset(mainFile, (uint)start);
                    var locCursor = tu.GetCursorForLocation(loc);
                    uint resultLength;
                    IDxcCursor[] cursors;
                    locCursor.FindReferencesInFile(mainFile, 0, 100, out resultLength, out cursors);

                    for (int i = 0; i < cursors.Length; ++i)
                    {
                        uint startOffset, endOffset;
                        GetRangeOffsets(cursors[i].GetExtent(), out startOffset, out endOffset);
                        data.Add((int)startOffset, (int)(endOffset - startOffset));
                    }
                    data.ApplyToRtb(rtb, Color.LightGray);
                    this.TheStatusStripLabel.Text = locCursor.GetCursorKind().ToString();
                }
            }
        }

        private static void GetRangeOffsets(IDxcSourceRange range, out uint start, out uint end)
        {
            IDxcSourceLocation l;
            IDxcFile file;
            uint line, col;
            l = range.GetStart();
            l.GetSpellingLocation(out file, out line, out col, out start);
            l = range.GetEnd();
            l.GetSpellingLocation(out file, out line, out col, out end);
        }

        private void CodeBox_TextChanged(object sender, EventArgs e)
        {
            if (this.DocKind == DocumentKind.CompiledObject)
                return;

            if (e != null)
                this.DocModified = true;
            this.InvalidateTU();
            if (this.AutoDisassemble)
                this.CompileDocument();
        }

        private string GetStringFromBlob(IDxcBlob blob)
        {
            return GetStringFromBlob(this.Library, blob);
        }

        public static string GetStringFromBlob(IDxcLibrary library, IDxcBlob blob)
        {
            unsafe
            {
                blob = library.GetBlobAstUf16(blob);
                return new string(blob.GetBufferPointer(), 0, (int)(blob.GetBufferSize() / 2));
            }
        }

        private byte[] GetBytesFromBlob(IDxcBlob blob)
        {
            unsafe
            {
                byte* pMem = (byte*)blob.GetBufferPointer();
                uint size = blob.GetBufferSize();
                byte[] result = new byte[size];
                fixed (byte* pTarget = result)
                {
                    for (uint i = 0; i < size; ++i)
                        pTarget[i] = pMem[i];
                }
                return result;
            }
        }

        private void selectAllToolStripMenuItem_Click(object sender, EventArgs e)
        {
            TextBoxBase tb = this.DeepActiveControl as TextBoxBase;
            if (tb != null)
            {
                tb.SelectAll();
                return;
            }
        }

        private void undoToolStripMenuItem_Click(object sender, EventArgs e)
        {
            TextBoxBase tb = this.DeepActiveControl as TextBoxBase;
            if (tb != null)
            {
                tb.Undo();
                return;
            }
        }

        private void cutToolStripMenuItem_Click(object sender, EventArgs e)
        {
            TextBoxBase tb = this.DeepActiveControl as TextBoxBase;
            if (tb != null)
            {
                tb.Cut();
                return;
            }
        }

        private void copyToolStripMenuItem_Click(object sender, EventArgs e)
        {
            TextBoxBase tb = this.DeepActiveControl as TextBoxBase;
            if (tb != null)
            {
                tb.Copy();
                return;
            }

            ListBox lb = this.DeepActiveControl as ListBox;
            if (lb != null)
            {
                if (lb.SelectedItems.Count > 0)
                {
                    string content;
                    if (lb.SelectedItems.Count == 1)
                    {
                        content = lb.SelectedItem.ToString();
                    }
                    else
                    {
                        StringBuilder sb = new StringBuilder();
                        foreach (var item in lb.SelectedItems)
                        {
                            sb.AppendLine(item.ToString());
                        }
                        content = sb.ToString();
                    }
                    Clipboard.SetText(content, TextDataFormat.UnicodeText);
                }
                return;
            }
        }

        private Control DeepActiveControl
        {
            get
            {
                Control result = this;
                while (result is IContainerControl)
                {
                    result = ((IContainerControl)result).ActiveControl;
                }
                return result;
            }
        }

        private void pasteToolStripMenuItem_Click(object sender, EventArgs e)
        {
            RichTextBox rtb = this.DeepActiveControl as RichTextBox;
            if (rtb != null)
            {
                // Handle the container format.
                if (Clipboard.ContainsData(ContainerData.DataFormat.Name))
                {
                    object o = Clipboard.GetData(ContainerData.DataFormat.Name);
                    rtb.SelectedText = ContainerData.DataObjectToString(o);
                }
                else
                {
                    rtb.Paste(DataFormats.GetFormat(DataFormats.UnicodeText));
                }
                return;
            }
            TextBoxBase tb = this.ActiveControl as TextBoxBase;
            if (tb != null)
            {
                tb.Paste();
                return;
            }
        }

        private void deleteToolStripMenuItem_Click(object sender, EventArgs e)
        {
            TextBoxBase tb = this.DeepActiveControl as TextBoxBase;
            if (tb != null)
            {
                tb.SelectedText = "";
                return;
            }
        }

        private void goToToolStripMenuItem_Click(object sender, EventArgs e)
        {
            int lastIndex = this.CodeBox.TextLength;
            int lastLine = this.CodeBox.GetLineFromCharIndex(lastIndex - 1);
            int currentLine = this.CodeBox.GetLineFromCharIndex(this.CodeBox.SelectionStart);
            using (GoToDialog dialog = new GoToDialog())
            {
                dialog.MaxLineNumber = lastLine + 1;
                dialog.LineNumber = currentLine + 1;
                if (dialog.ShowDialog(this) == DialogResult.OK)
                {
                    this.CodeBox.Select(this.CodeBox.GetFirstCharIndexFromLine(dialog.LineNumber - 1), 0);
                }
            }
        }

        private void ResetDefaultPassesButton_Click(object sender, EventArgs e)
        {
            this.ResetDefaultPasses();
        }

        private static bool IsDisassemblyTokenChar(char c)
        {
            return char.IsLetterOrDigit(c) || c == '@' || c == '%' || c == '$';
        }

        private static bool IsTokenLeftBoundary(string text, int i)
        {
            // Whether there is a token boundary between text[i] and text[i-1].
            if (i == 0) return true;
            if (i >= text.Length - 1) return true;
            char cPrior = text[i - 1];
            char c = text[i];
            return !IsDisassemblyTokenChar(cPrior) && IsDisassemblyTokenChar(c);
        }

        private static bool IsTokenRightBoundary(string text, int i)
        {
            if (i == 0) return true;
            if (i >= text.Length - 1) return true;
            char cPrior = text[i - 1];
            char c = text[i];
            return IsDisassemblyTokenChar(cPrior) && !IsDisassemblyTokenChar(c);
        }

        private static int ColorToCOLORREF(Color value)
        {
            // 0x00bbggrr.
            int result = value.R | (value.G << 8) | (value.B << 16);
            return result;
        }

        private static Tom.ITextRange SetStartLengthColor(Tom.ITextDocument doc, int start, int length, Color color)
        {
            Tom.ITextRange range = doc.Range(start, start + length);
            Tom.ITextFont font = range.Font;
            font.ForeColor = ColorToCOLORREF(color);
            return range;
        }

        private static void SetStartLengthBackColor(Tom.ITextRange range, Color color)
        {
            Tom.ITextFont font = range.Font;
            font.BackColor = ColorToCOLORREF(color);
        }

        private static Tom.ITextRange SetStartLengthBackColor(Tom.ITextDocument doc, int start, int length, Color color)
        {
            Tom.ITextRange range = doc.Range(start, start + length);
            SetStartLengthBackColor(range, color);
            return range;
        }

        private static Regex dbgLineRegEx;
        private static Regex DbgLineRegEx { get { return (dbgLineRegEx = dbgLineRegEx ?? new Regex(@"line: (\d+)")); } }
        private static Regex dbgColRegEx;
        private static Regex DbgColRegEx { get { return (dbgColRegEx = dbgColRegEx ?? new Regex(@"column: (\d+)")); } }

        private static void HandleDebugMetadata(string dbgLine, RichTextBox sourceBox)
        {
            Match lineMatch = DbgLineRegEx.Match(dbgLine);
            if (!lineMatch.Success)
            {
                return;
            }

            int lineVal = Int32.Parse(lineMatch.Groups[1].Value) - 1;
            int targetStart = sourceBox.GetFirstCharIndexFromLine(lineVal);
            int targetEnd = sourceBox.GetFirstCharIndexFromLine(lineVal + 1);
            Match colMatch = DbgColRegEx.Match(dbgLine);
            if (colMatch.Success)
            {
                targetStart += Int32.Parse(colMatch.Groups[1].Value) - 1;
            }

            var highlights = SelectionHighlightData.FromRtb(sourceBox);
            highlights.ClearFromRtb(sourceBox);
            highlights.Add(targetStart, targetEnd - targetStart);
            highlights.ApplyToRtb(sourceBox, Color.Yellow);

            sourceBox.SelectionStart = targetStart;
            sourceBox.ScrollToCaret();
        }

        private static void HandleDebugTokenOnDisassemblyLine(RichTextBox rtb, RichTextBox sourceBox)
        {
            // Get the line.
            string[] lines = rtb.Lines;
            string line = lines[rtb.GetLineFromCharIndex(rtb.SelectionStart)];
            Regex re = new Regex(@"!dbg !(\d+)");
            Match m = re.Match(line);
            if (!m.Success)
            {
                return;
            }

            string val = m.Groups[1].Value;
            int dbgMetadata = Int32.Parse(val);
            for (int dbgLineIndex = lines.Length - 1; dbgLineIndex >= 0;)
            {
                string dbgLine = lines[dbgLineIndex];
                if (dbgLine.StartsWith("!"))
                {
                    int dbgIdx = Int32.Parse(dbgLine.Substring(1, dbgLine.IndexOf(' ') - 1));
                    if (dbgIdx == dbgMetadata)
                    {
                        HandleDebugMetadata(dbgLine, sourceBox);
                        return;
                    }
                    else if (dbgIdx < dbgMetadata)
                    {
                        return;
                    }
                    else
                    {
                        dbgLineIndex -= (dbgIdx - dbgMetadata);
                    }
                }
                else
                {
                    --dbgLineIndex;
                }
            }
        }

        public static void HandleCodeSelectionChanged(RichTextBox rtb, RichTextBox sourceBox)
        {
            SelectionHighlightData data = SelectionHighlightData.FromRtb(rtb);
            SelectionExpandResult expand = SelectionExpandResult.Expand(rtb);
            if (expand.IsEmpty)
                return;

            string token = expand.Token;
            if (sourceBox != null && token == "dbg")
            {
                HandleDebugTokenOnDisassemblyLine(rtb, sourceBox);
            }

            if (data.SelectedToken == token)
                return;
            string text = expand.Text;

            // OK, time to do work.
            using (new RichTextBoxEditAction(rtb))
            {
                data.SelectedToken = token;
                data.ClearFromRtb(rtb);

                int match = text.IndexOf(token);
                while (match != -1)
                {
                    data.Add(match, token.Length);
                    match += token.Length;
                    match = text.IndexOf(token, match);
                }
                data.ApplyToRtb(rtb, Color.LightPink);
            }
        }

        private void DisassemblyTextBox_SelectionChanged(object sender, EventArgs e)
        {
            HandleCodeSelectionChanged((RichTextBox)sender, this.CodeBox);
        }


        private string PassToPassString(IDxcOptimizerPass pass)
        {
            return pass.GetOptionName() + OptDescSeparator + pass.GetDescription();
        }

        private string PassStringToBanner(string value)
        {
            int separator = value.IndexOf(OptDescSeparator);
            if (separator >= 0)
                value = value.Substring(0, separator);
            return value;
        }

        private static string PassStringToOption(string value)
        {
            int separator = value.IndexOf(OptDescSeparator);
            if (separator >= 0)
                value = value.Substring(0, separator);
            return "-" + value;
        }

        private void AnalysisTabControl_Selecting(object sender, TabControlCancelEventArgs e)
        {
            if (e.TabPage == this.OptimizerTabPage)
            {
                if (passesLoaded)
                {
                    return;
                }

                this.ResetDefaultPasses();
            }
            if (e.TabPage == this.ASTTabPage)
            {
                if (pendingASTDump != null)
                {
                    pendingASTDump();
                    pendingASTDump = null;
                }
            }
        }

        private void ResetDefaultPasses()
        {
            IDxcOptimizer opt = HlslDxcLib.CreateDxcOptimizer();
            if (!this.passesLoaded)
            {
                int passCount = opt.GetAvailablePassCount();
                PassInfo[] localInfos = new PassInfo[passCount];
                for (int i = 0; i < passCount; ++i)
                {
                    localInfos[i] = PassInfo.FromOptimizerPass(opt.GetAvailablePass(i));
                }
                localInfos = localInfos.OrderBy(p => p.Name).ToArray();
                this.passInfos = localInfos.ToList();
                this.AvailablePassesBox.Items.AddRange(localInfos);
                this.passesLoaded = true;
            }

            List<string> args;
            try
            {
                HlslFileVariables fileVars = GetFileVars();
                args = fileVars.Arguments.Where(a => !String.IsNullOrWhiteSpace(a)).ToList();
            }
            catch (Exception)
            {
                args = new List<string>() { "/Od" };
            }

            args.Add("/Odump");
            IDxcCompiler compiler = HlslDxcLib.CreateDxcCompiler();
            IDxcOperationResult optDumpResult =
                compiler.Compile(CreateBlobForText("[RootSignature(\"\")]float4 main() : SV_Target { return 0; }"), "hlsl.hlsl", "main", "ps_6_0", args.ToArray(), args.Count, null, 0, null);
            IDxcBlob optDumpBlob = optDumpResult.GetResult();
            string optDumpText = GetStringFromBlob(optDumpBlob);
            this.AddSelectedPassesFromText(optDumpText, true);
        }

        private void AddSelectedPassesFromText(string optDumpText, bool replace)
        {
            List<object> defaultPasses = new List<object>();
            foreach (string line in optDumpText.Split('\n'))
            {
                if (line.StartsWith("#")) continue;
                string lineTrim = line.Trim();
                if (String.IsNullOrEmpty(lineTrim)) continue;
                lineTrim = line.TrimStart('-');
                int argSepIndex = lineTrim.IndexOf(',');
                string passName = argSepIndex > 0 ? lineTrim.Substring(0, argSepIndex) : lineTrim;
                PassInfo passInfo = this.passInfos.FirstOrDefault(p => p.Name == passName);
                if (passInfo == null)
                {
                    defaultPasses.Add(lineTrim);
                    continue;
                }
                PassInfoWithValues passWithValues = new PassInfoWithValues(passInfo);
                if (argSepIndex > 0)
                {
                    bool problemFound = false;
                    string[] parts = lineTrim.Split(',');
                    for (int i = 1; i < parts.Length; ++i)
                    {
                        string[] nameValue = parts[i].Split('=');
                        PassArgInfo argInfo = passInfo.Args.FirstOrDefault(a => a.Name == nameValue[0]);
                        if (argInfo == null)
                        {
                            problemFound = true;
                            break;
                        }
                        passWithValues.Values.Add(new PassArgValueInfo()
                        {
                            Arg = argInfo,
                            Value = (nameValue.Length == 1) ? null : nameValue[1]
                        });
                    }
                    if (problemFound)
                    {
                        defaultPasses.Add(lineTrim);
                        continue;
                    }
                }
                defaultPasses.Add(passWithValues);
            }

            if (replace)
            {
                this.SelectedPassesBox.Items.Clear();
            }
            this.SelectedPassesBox.Items.AddRange(defaultPasses.ToArray());
        }

        public static string[] CreatePassOptions(IEnumerable<string> passes, bool analyze, bool printAll)
        {
            List<string> result = new List<string>();
            if (analyze)
                result.Add("-analyze");
            if (printAll)
                result.Add("-print-module:start");
            bool inOptFn = false;
            foreach (var itemText in passes)
            {
                result.Add(PassStringToOption(itemText));
                if (itemText == "opt-fn-passes")
                {
                    inOptFn = true;
                }
                else if (itemText.StartsWith("opt-") && itemText.EndsWith("-passes"))
                {
                    inOptFn = false;
                }
                if (printAll && !inOptFn)
                {
                    result.Add("-hlsl-passes-pause");
                    result.Add("-print-module:" + itemText);
                }
            }
            return result.ToArray();
        }

        private string[] CreatePassOptions(bool analyze, bool printAll)
        {
            return CreatePassOptions(this.SelectedPassesBox.Items.ToStringEnumerable(), analyze, printAll);
        }

        private void RunPassesButton_Click(object sender, EventArgs e)
        {
            // TODO: consider accepting DXIL in the code editor as well
            // Do a high-level only compile.
            HighLevelCompileResult compile = RunHighLevelCompile();
            if (compile.Blob == null)
            {
                MessageBox.Show("Failed to compile: " + compile.ResultText);
                return;
            }

            string[] options = CreatePassOptions(this.AnalyzeCheckBox.Checked, false);
            OptimizeResult opt = RunOptimize(this.Library, options, compile.Blob);
            if (!opt.Succeeded)
            {
                MessageBox.Show("Failed to optimize: " + opt.ResultText);
                return;
            }

            Form form = new Form();
            RichTextBox rtb = new RichTextBox();
            LogContextMenuHelper helper = new LogContextMenuHelper(rtb);
            rtb.Dock = DockStyle.Fill;
            rtb.Font = this.CodeBox.Font;
            rtb.ContextMenu = new ContextMenu(
                new MenuItem[]
                {
                    new MenuItem("Show Graph", helper.ShowGraphClick)
                });
            rtb.SelectionChanged += DisassemblyTextBox_SelectionChanged;
            rtb.Text = opt.ResultText;
            form.Controls.Add(rtb);
            form.StartPosition = FormStartPosition.CenterParent;
            form.Show(this);
        }

        private void SelectPassUpButton_Click(object sender, EventArgs e)
        {
            ListBox lb = this.SelectedPassesBox;
            int selectedIndex = lb.SelectedIndex;
            if (selectedIndex == -1 || selectedIndex == 0)
                return;
            object o = lb.Items[selectedIndex - 1];
            lb.Items.RemoveAt(selectedIndex - 1);
            lb.Items.Insert(selectedIndex, o);
        }

        private void SelectPassDownButton_Click(object sender, EventArgs e)
        {
            ListBox lb = this.SelectedPassesBox;
            int selectedIndex = lb.SelectedIndex;
            if (selectedIndex == -1 || selectedIndex == lb.Items.Count - 1)
                return;
            object o = lb.Items[selectedIndex + 1];
            lb.Items.RemoveAt(selectedIndex + 1);
            lb.Items.Insert(selectedIndex, o);
        }

        private void AvailablePassesBox_DoubleClick(object sender, EventArgs e)
        {
            ListBox lb = (ListBox)sender;
            if (lb.SelectedItems.Count == 0)
                return;
            foreach (var item in lb.SelectedItems)
                this.SelectedPassesBox.Items.Add(new PassInfoWithValues((PassInfo)item));
        }

        private void SelectedPassesBox_DoubleClick(object sender, EventArgs e)
        {
                for (int x = SelectedPassesBox.SelectedIndices.Count - 1; x >= 0; x--)
                {
                    int idx = SelectedPassesBox.SelectedIndices[x];
                    SelectedPassesBox.Items.RemoveAt(idx);
                }

        }

        private void AddPrintModuleButton_Click(object sender, EventArgs e)
        {
            // Known, very handy.
            this.SelectedPassesBox.Items.Add("print-module" + OptDescSeparator + "Print module to stderr");
        }

        private void SelectedPassesBox_KeyUp(object sender, KeyEventArgs e)
        {
            ListBox lb = (ListBox)sender;
            if (e.KeyCode == Keys.Delete)
            {
                for (int x = SelectedPassesBox.SelectedIndices.Count - 1; x >= 0; x--)
                {
                    int idx = SelectedPassesBox.SelectedIndices[x];
                    SelectedPassesBox.Items.RemoveAt(idx);
                }
                e.Handled = true;
                return;
            }
        }

        private void autoUpdateToolStripMenuItem_Click(object sender, EventArgs e)
        {
            this.AutoDisassemble = !this.AutoDisassemble;
        }

        private static string GetValueOrDefault(Dictionary<string, string> d, string name, string defaultValue)
        {
            string result;
            if (!d.TryGetValue(name, out result))
                result = defaultValue;
            return result;
        }

        /// <summary>Helper class to handle the context menu of operations log.</summary>
        public class LogContextMenuHelper
        {
            public RichTextBox Rtb { get; set; }

            public LogContextMenuHelper(RichTextBox rtb)
            {
                this.Rtb = rtb;
            }

            public void ShowGraphClick(object sender, EventArgs e)
            {
                SelectionExpandResult s = SelectionExpandResult.Expand(this.Rtb);
                if (s.IsEmpty) return;
                if (s.Token == "digraph")
                {
                    int nextStart = s.Text.IndexOf('{', s.SelectionEnd);
                    if (nextStart < 0) return;
                    int closing = FindBalanced('{', '}', s.Text, nextStart);
                    if (closing < 0) return;

                    // See file history for a version that inserted the image in-line with graph.
                    // The svg/web browser approach provides zooming and more interactivity.
                    string graphText = s.Text.Substring(s.SelectionStart, closing - s.SelectionStart);
                    ShowDot(graphText);
                }
            }

            static public void ShowDot(string graphText)
            {
                string path = System.IO.Path.GetTempFileName();
                string outPath = path + ".svg";
                try
                {
                    System.IO.File.WriteAllText(path, graphText);

                    string svgData = RunDot(path, DotOutFormat.Svg, null);
                    Form browserForm = new Form();

                    TrackBar zoomControl = new TrackBar();
                    zoomControl.Minimum = 10;
                    zoomControl.Maximum = 400;
                    zoomControl.Value = 100;
                    zoomControl.Dock = DockStyle.Top;
                    zoomControl.Text = "zoom";

                    WebBrowser browser = new WebBrowser();
                    browser.Dock = DockStyle.Fill;
                    browser.DocumentText = svgData;
                    browserForm.Controls.Add(browser);
                    browserForm.Controls.Add(zoomControl);
                    zoomControl.ValueChanged += (_, __) =>
                    {
                        if (browser.Document != null && browser.Document.DomDocument != null)
                        {
                            dynamic o = browser.Document.DomDocument;
                            o.documentElement.style.zoom = String.Format("{0}%", zoomControl.Value);
                        }
                    };
                    browserForm.Text = "graph";
                    browserForm.Show();
                }
                catch (DotProgram.CannotFindDotException cfde)
                {
                    MessageBox.Show(cfde.Message, "Unable to find dot.exe", MessageBoxButtons.OK, MessageBoxIcon.Error);
                }
                finally
                {
                    DeleteIfExists(path);
                }
            }

            internal static void DeleteIfExists(string path)
            {
                if (System.IO.File.Exists(path))
                    System.IO.File.Delete(path);
            }

            internal static string RunDot(string inputFile, DotOutFormat format, string outFile)
            {
                DotProgram program = new DotProgram();
                program.InFilePath = inputFile;
                program.OutFilePath = outFile;
                program.OutFormat = format;
                return program.StartAndWaitForExit();
            }

            internal static int FindBalanced(char open, char close, string text, int start)
            {
                // return exclusive end
                System.Diagnostics.Debug.Assert(text[start] == open);
                int level = 1;
                int result = start + 1;
                int end = text.Length;
                while (result < end && level != 0)
                {
                    if (text[result] == open) level++;
                    if (text[result] == close) level--;
                    result++;
                }
                return (result == end) ? -1 : result;
            }
        }

        internal enum DotOutFormat
        {
            Svg,
            Png
        }

        class DotProgram
        {
            private string fileName;
            private Dictionary<string, string> options;

            internal class CannotFindDotException : InvalidOperationException
            {
                internal CannotFindDotException(string message) : base(message) { }
            }

            public DotProgram()
            {
                this.options = new Dictionary<string, string>();
                this.options["-Nfontname"] = "Consolas";
                this.options["-Efontname"] = "Tahoma";
            }

            public static IEnumerable<string> DotFileNameCandidates()
            {
                // Look in a few known places.
                string path = Environment.GetEnvironmentVariable("PATH");
                string[] partPaths = path.Split(';');
                foreach (var partPath in partPaths)
                {
                    yield return System.IO.Path.Combine(partPath, "bin\\dot.exe");
                }

                string progPath = Environment.GetFolderPath(Environment.SpecialFolder.ProgramFilesX86);
                if (!String.IsNullOrEmpty(progPath))
                {
                    yield return System.IO.Path.Combine(progPath, "Graphviz2.38\\bin\\dot.exe");
                }

                progPath = Environment.GetFolderPath(Environment.SpecialFolder.ProgramFiles);
                yield return System.IO.Path.Combine(progPath, "Graphviz2.38\\bin\\dot.exe");
            }

            public static string FindDotFileName()
            {
                StringBuilder sb = new StringBuilder();
                sb.AppendLine("Cannot find dot.exe in any of these locations.");
                foreach (string result in DotFileNameCandidates())
                {
                    sb.AppendLine(result);
                    if (System.IO.File.Exists(result))
                        return result;
                }
                throw new CannotFindDotException(sb.ToString());
            }

            public string BuildArguments()
            {
                StringBuilder sb = new StringBuilder();
                sb.AppendFormat("-T{0}", this.OutFormat.ToString().ToLowerInvariant());
                foreach (var pair in this.Options)
                {
                    sb.AppendFormat(" {0}={1}", pair.Key, pair.Value);
                }
                if (!String.IsNullOrEmpty(this.OutFilePath))
                {
                    sb.AppendFormat(" -o{0}", this.OutFilePath);
                }
                sb.AppendFormat(" {0}", this.InFilePath);
                return sb.ToString();
            }

            public string FileName
            {
                get
                {
                    if (String.IsNullOrEmpty(this.fileName))
                    {
                        this.fileName = FindDotFileName();
                    }
                    return this.fileName;
                }
                set
                {
                    this.fileName = value;
                }
            }
            public string InFilePath { get; set; }
            public IDictionary<string, string> Options
            {
                get { return this.options; }
            }
            public string OutFilePath { get; set; }
            public DotOutFormat OutFormat { get; set; }
            public System.Diagnostics.Process Start()
            {
                System.Diagnostics.ProcessStartInfo psi = new System.Diagnostics.ProcessStartInfo(this.FileName, this.BuildArguments());
                psi.CreateNoWindow = true;
                psi.RedirectStandardOutput = String.IsNullOrEmpty(this.OutFilePath);
                psi.UseShellExecute = false;
                return System.Diagnostics.Process.Start(psi);
            }
            public string StartAndWaitForExit()
            {
                using (System.Diagnostics.Process p = this.Start())
                {
                    string result = p.StandardOutput.ReadToEnd();
                    p.WaitForExit();
                    int code = p.ExitCode;
                    if (code > 0)
                    {
                        throw new Exception("dot.exe failed with code " + code);
                    }
                    return result;
                }
            }
        }

        /// <summary>Helper class to expand a short selection into something more useful.</summary>
        class SelectionExpandResult
        {
            public int SelectionStart { get; set; }
            public int SelectionEnd { get; set; }
            public string Text { get; set; }
            public string Token { get; set; }

            public bool IsEmpty
            {
                get { return SelectionEnd - 1 == SelectionStart; }
            }

            internal static SelectionExpandResult Empty()
            {
                return new SelectionExpandResult() { SelectionStart = 0, SelectionEnd = 1 };
            }

            internal static SelectionExpandResult Expand(RichTextBox rtb)
            {
                string text = rtb.Text;
                int selStart = rtb.SelectionStart;
                int selLength = rtb.SelectionLength;
                int tokenStart = selStart;
                int tokenEnd = selStart;
                if (tokenStart < text.Length && !char.IsLetterOrDigit(text[tokenStart]))
                    return Empty();
                // check last token case
                tokenEnd++; // it's a letter or digit, so it's at least one offset
                while (tokenStart > 0 && !IsTokenLeftBoundary(text, tokenStart))
                    tokenStart--;
                while (tokenEnd < text.Length && !IsTokenRightBoundary(text, tokenEnd))
                    tokenEnd++;

                if (tokenEnd - 1 == tokenStart)
                    return Empty();

                string token = text.Substring(tokenStart, tokenEnd - tokenStart);
                return new SelectionExpandResult()
                {
                    SelectionEnd = tokenEnd,
                    SelectionStart = tokenStart,
                    Text = text,
                    Token = token
                };
            }
        }

        /// <summary>Helper class to record editor highlights.</summary>
        class SelectionHighlightData
        {
            public static SelectionHighlightData FromRtb(RichTextBox rtb)
            {
                SelectionHighlightData result = (SelectionHighlightData)rtb.Tag;
                if (result == null)
                {
                    result = new SelectionHighlightData();
                    rtb.Tag = result;
                }
                return result;
            }

            public static void ClearAnyFromRtb(RichTextBox rtb)
            {
                SelectionHighlightData data = (SelectionHighlightData)rtb.Tag;
                if (data != null)
                    data.ClearFromRtb(rtb);
            }

            public void Add(int start, int length)
            {
                this.StartLengthHighlights.Add(new Tuple<int, int>(start, length));
            }

            public void ApplyToRtb(RichTextBox rtb, Color color)
            {
                Tom.ITextDocument doc = GetTextDocument(rtb);
                foreach (var pair in this.StartLengthHighlights)
                {
                    this.HighlightRanges.Add(SetStartLengthBackColor(doc, pair.Item1, pair.Item2, color));
                }
            }

            public void ClearFromRtb(RichTextBox rtb)
            {
                Tom.ITextDocument doc = GetTextDocument(rtb);
                for (int i = 0; i < this.HighlightRanges.Count; ++i)
                {
                    SetStartLengthBackColor(HighlightRanges[i], rtb.BackColor);
                }
                this.StartLengthHighlights.Clear();
                this.HighlightRanges.Clear();
            }

            private List<Tom.ITextRange> HighlightRanges = new List<Tom.ITextRange>();
            public List<Tuple<int, int>> StartLengthHighlights = new List<Tuple<int, int>>();
            public string SelectedToken;
        }

        [System.Runtime.InteropServices.Guid("00020d00-0000-0000-c000-000000000046")]
        interface IRichEditOle
        {

        }

        private const int WM_USER = 0x0400;
        private const int EM_GETOLEINTERFACE = (WM_USER + 60);
        [System.Runtime.InteropServices.DllImport(
            "user32.dll", CharSet = System.Runtime.InteropServices.CharSet.Auto)]
        private static extern IntPtr SendMessage(IntPtr hWnd, int msg, IntPtr wParam, ref IRichEditOle lParam);
        internal static Tom.ITextDocument GetTextDocument(RichTextBox rtb)
        {
            IRichEditOle ole = null;
            SendMessage(rtb.Handle, EM_GETOLEINTERFACE, IntPtr.Zero, ref ole);
            return ole as Tom.ITextDocument;
        }

        /// <summary>Helper class to suppress events and restore selection.</summary>
        class RichTextBoxEditAction : IDisposable
        {
            private const int EM_SETEVENTMASK = (WM_USER + 69);
            private const int EM_GETSCROLLPOS = (WM_USER + 221);
            private const int EM_SETSCROLLPOS = (WM_USER + 222);
            private const int WM_SETREDRAW = 0x0b;
            private RichTextBox rtb;
            private bool readOnly;
            private IntPtr eventMask;

            [System.Runtime.InteropServices.DllImport(
                "user32.dll", CharSet = System.Runtime.InteropServices.CharSet.Auto)]
            private static extern IntPtr SendMessage(IntPtr hWnd, int msg, IntPtr wParam, IntPtr lParam);
            [System.Runtime.InteropServices.DllImport(
                "user32.dll", CharSet = System.Runtime.InteropServices.CharSet.Auto)]
            private static extern IntPtr SendMessage(IntPtr hWnd, int msg, IntPtr wParam, ref Point lParam);

            internal RichTextBoxEditAction(RichTextBox rtb)
            {
                this.rtb = rtb;
                this.readOnly = rtb.ReadOnly;
                this.eventMask = (IntPtr)SendMessage(rtb.Handle, EM_SETEVENTMASK, IntPtr.Zero, IntPtr.Zero);
                SendMessage(rtb.Handle, WM_SETREDRAW, IntPtr.Zero, IntPtr.Zero);
                this.rtb.ReadOnly = false;
            }

            public void Dispose()
            {
                SendMessage(rtb.Handle, WM_SETREDRAW, (IntPtr)1, IntPtr.Zero);
                SendMessage(rtb.Handle, EM_SETEVENTMASK, IntPtr.Zero, this.eventMask);
                this.rtb.ReadOnly = this.readOnly;
            }
        }

        private void SelectNodeWithOffset(TreeView view, TreeNode node, int offset)
        {
            bool foundBetter;
            do
            {
                foundBetter = false;
                foreach (TreeNode child in node.Nodes)
                {
                    TreeNodeRange r = child.Tag as TreeNodeRange;
                    if (r != null && r.Contains(offset))
                    {
                        node = child;
                        foundBetter = true;
                        break;
                    }
                }
            }
            while (foundBetter);
            view.SelectedNode = node;
        }

        private void bitstreamToolStripMenuItem_Click(object sender, EventArgs e)
        {
            if (this.SelectedShaderBlob == null)
            {
                MessageBox.Show(this, "No shader blob selected. Try compiling a file.");
                return;
            }

            DisplayBitstream(ContainerData.BlobToBytes(this.SelectedShaderBlob), "Bitstream Viewer - Selected Shader");
        }

        private void bitstreamFromClipboardToolStripMenuItem_Click(object sender, EventArgs e)
        {
            if (!Clipboard.ContainsData(ContainerData.DataFormat.Name))
            {
                MessageBox.Show(this, "No shader blob on clipboard. Try pasting from the optimizer view.");
                return;
            }

            DisplayBitstream(ContainerData.DataObjectToBytes(Clipboard.GetData(ContainerData.DataFormat.Name)),
                "Bitstream Viewer - Clipboard");
        }

        private void DisplayBitstream(byte[] bytes, string title)
        {
            StatusBar statusBar = new StatusBar();
            statusBar.Dock = DockStyle.Bottom;

            BinaryViewControl binaryView = new BinaryViewControl();

            TreeView treeView = new TreeView();
            treeView.Dock = DockStyle.Fill;
            treeView.AfterSelect += (eSender, treeViewEventArgs) =>
            {
                TreeNodeRange r = treeViewEventArgs.Node.Tag as TreeNodeRange;
                if (r == null)
                {
                    binaryView.SetSelection(0, 0);
                    statusBar.ResetText();
                }
                else
                {
                    binaryView.SetSelection(r.Offset, r.Length);
                    statusBar.Text = String.Format("Bits {0}-{1} (length {2})", r.Offset, r.Offset + r.Length, r.Length);
                }
            };
            BuildBitstreamNodes(bytes, treeView.Nodes);

            binaryView.BitClick += (eSender, bitArgs) =>
            {
                int offset = bitArgs.BitOffset;
                SelectNodeWithOffset(treeView, treeView.Nodes[0], offset);
            };
            binaryView.BitMouseMove += (eSender, bitArgs) =>
            {
                int offset = bitArgs.BitOffset;
                int byteOffset = offset / 8;
                byte b = bytes[byteOffset];
                string toolTipText = String.Format("Byte @ 0x{0:x} = 0x{1:x} = {2}", byteOffset, b, b);
                TheToolTip.SetToolTip(binaryView, toolTipText);
            };

            Panel binaryPanel = new Panel();
            binaryPanel.Dock = DockStyle.Fill;
            binaryPanel.AutoScroll = true;
            binaryPanel.Controls.Add(binaryView);

            SplitContainer container = new SplitContainer();
            container.Orientation = Orientation.Vertical;
            container.Panel1.Controls.Add(treeView);
            container.Panel2.Controls.Add(binaryPanel);
            container.Dock = DockStyle.Fill;

            Form form = new Form();
            form.Text = title;
            form.Controls.Add(container);
            form.Controls.Add(statusBar);
            binaryView.Bytes = bytes;
            form.StartPosition = FormStartPosition.CenterParent;
            form.Show(this);
        }

        #region Bitstream generation.

        private static void BuildBitstreamNodes(byte[] bytes, TreeNodeCollection nodes)
        {
            TreeNode root;
            if (bytes[0] == 'D' && bytes[1] == 'X' && bytes[2] == 'B' && bytes[3] == 'C')
            {
                root = RangeNode("Content: DXBC");
                BuildBitstreamForDXBC(bytes, root);
            }
            else
            {
                root = RangeNode("Content: Unknown", 0, bytes.Length);
            }
            nodes.Add(root);
            AddBitstreamOffsets(root);
        }

        private static void AddBitstreamOffsets(TreeNode node)
        {
            TreeNodeRange r = node.Tag as TreeNodeRange;
            if (r == null)
            {
                int offset = -1;
                int length = 0;
                foreach (TreeNode child in node.Nodes)
                {
                    AddBitstreamOffsets(child);
                    TreeNodeRange childRange = child.Tag as TreeNodeRange;
                    Debug.Assert(childRange != null);
                    if (offset == -1)
                    {
                        offset = childRange.Offset;
                        length = childRange.Length;
                    }
                    else
                    {
                        Debug.Assert(offset <= childRange.Offset);
                        int lastBit = childRange.Offset + childRange.Length;
                        length = Math.Max(length, lastBit - offset);
                    }
                }
                node.Tag = new TreeNodeRange(offset, length);
            }
        }

        private static void BuildBitstreamForDXBC(byte[] bytes, TreeNode root)
        {
            int offset = 0;
            TreeNode header = RangeNode("Header");
            root.Nodes.Add(header);
            string signature;
            header.Nodes.Add(RangeNodeASCII(bytes, "Signature", ref offset, 4, out signature));
            header.Nodes.Add(RangeNodeBytes("Hash", ref offset, 16 * 8));
            ushort verMajor, verMinor;
            header.Nodes.Add(RangeNodeUInt16(bytes, "VerMajor", ref offset, out verMajor));
            header.Nodes.Add(RangeNodeUInt16(bytes, "VerMinor", ref offset, out verMinor));
            uint containerSize, partCount;
            header.Nodes.Add(RangeNodeUInt32(bytes, "ContainerSize", ref offset, out containerSize));
            header.Nodes.Add(RangeNodeUInt32(bytes, "PartCount", ref offset, out partCount));

            uint[] partOffsets = new uint[partCount];
            TreeNode partOffsetTable = RangeNode("Part Offsets");
            root.Nodes.Add(partOffsetTable);
            for (uint i = 0; i < partCount; i++)
            {
                uint partSize;
                partOffsetTable.Nodes.Add(RangeNodeUInt32(bytes, "Part Offset #" + i, ref offset, out partSize));
                partOffsets[i] = partSize;
            }

            TreeNode partsNode = RangeNode("Parts");
            root.Nodes.Add(partsNode);
            for (uint i = 0; i < partCount; i++)
            {
                offset = (int)(8 * partOffsets[i]);
                TreeNode partNode = RangeNode("Part #" + i);
                TreeNode headerNode = RangeNode("Header");
                string partCC;
                UInt32 partSize;
                headerNode.Nodes.Add(RangeNodeASCII(bytes, "PartFourCC", ref offset, 4, out partCC));
                headerNode.Nodes.Add(RangeNodeUInt32(bytes, "PartSize", ref offset, out partSize));
                partNode.Nodes.Add(headerNode);
                if (partCC == "DXIL")
                {
                    BuildBitstreamForDXIL(bytes, offset, partNode);
                }
                else if (partCC == "ISGN" || partCC == "OSGN" || partCC == "PSGN" || partCC == "ISG1" || partCC == "OSG1" || partCC == "PSG1")
                {
                    BuildBitstreamForSignature(bytes, offset, partNode, partCC);
                }
                partsNode.Nodes.Add(partNode);
            }
        }

        private static void BuildBitstreamForSignature(byte[] bytes, int offset, TreeNode root, string partName)
        {
            // DxilProgramSignature
            bool hasStream = partName.Last() == '1';
            bool hasMinprec = hasStream;
            int startOffset = offset;
            uint paramCount, paramOffset;
            root.Nodes.Add(RangeNodeUInt32(bytes, "ParamCount", ref offset, out paramCount));
            root.Nodes.Add(RangeNodeUInt32(bytes, "ParamOffset", ref offset, out paramOffset));
            if (paramOffset != 8)
                return; // padding here not yet implemented

            for (int i = 0; i < paramCount; i++)
            {
                TreeNode paramNode = RangeNode("Param #" + i);

                // DxilProgramSignatureElement
                uint stream, semanticIndex, semanticName, systemValue, compType, register, minprec;
                ushort pad;
                byte mask, maskUsage;
                if (hasStream)
                    paramNode.Nodes.Add(RangeNodeUInt32(bytes, "Stream", ref offset, out stream));
                paramNode.Nodes.Add(RangeNodeUInt32(bytes, "SemanticName", ref offset, out semanticName));
                // Now go read the string.
                if (semanticName != 0)
                {
                    StringBuilder sb = new StringBuilder();
                    int nameOffset = startOffset / 8 + (int)semanticName;
                    while (bytes[nameOffset] != 0)
                    {
                        sb.Append((char)bytes[nameOffset]);
                        nameOffset++;
                    }
                    paramNode.Text += " - " + sb.ToString();
                }
                paramNode.Nodes.Add(RangeNodeUInt32(bytes, "SemanticIndex", ref offset, out semanticIndex));
                paramNode.Nodes.Add(RangeNodeUInt32(bytes, "SystemValue", ref offset, out systemValue));
                paramNode.Nodes.Add(RangeNodeUInt32(bytes, "CompType", ref offset, out compType));
                paramNode.Nodes.Add(RangeNodeUInt32(bytes, "Register", ref offset, out register));
                paramNode.Nodes.Add(RangeNodeUInt8(bytes, "Mask", ref offset, out mask));
                paramNode.Nodes.Add(RangeNodeUInt8(bytes, "MaskUsage", ref offset, out maskUsage));
                paramNode.Nodes.Add(RangeNodeUInt16(bytes, "Pad", ref offset, out pad));
                if (hasMinprec)
                {
                    paramNode.Nodes.Add(RangeNodeUInt32(bytes, "Minprecision", ref offset, out minprec));
                }

                root.Nodes.Add(paramNode);
            }
        }

        private static void BuildBitstreamForDXIL(byte[] bytes, int offset, TreeNode root)
        {
            TreeNode header = RangeNode("DxilProgramHeader");
            uint programVersion, programSize, dxilVersion, bcOffset, bcSize;
            string magic;
            TreeNode verNode = RangeNodeUInt32(bytes, "ProgramVersion", ref offset, out programVersion);
            verNode.Text += " - " + DescribeProgramVersion(programVersion);
            header.Nodes.Add(verNode);
            header.Nodes.Add(RangeNodeUInt32(bytes, "SizeInUint32", ref offset, out programSize));
            int programOffset = offset;
            header.Nodes.Add(RangeNodeASCII(bytes, "Magic", ref offset, 4, out magic));
            header.Nodes.Add(RangeNodeUInt32(bytes, "DXIL Version", ref offset, out dxilVersion));
            header.Nodes.Add(RangeNodeUInt32(bytes, "Bitcode Offset", ref offset, out bcOffset));
            header.Nodes.Add(RangeNodeUInt32(bytes, "Bitcode Size", ref offset, out bcSize));
            int bitcodeOffset = (int)(programOffset + bcOffset * 8);
            offset = bitcodeOffset;
            root.Nodes.Add(header);

            TreeNode bcNode = RangeNode("DXIL bitcode");
            try
            {
                DxilBitcodeReader.BuildTree(bytes, ref offset, (int)(bcSize * 8), bcNode);
            }
            catch (Exception e)
            {
                bcNode.Text += e.Message;
            }
            root.Nodes.Add(bcNode);
        }

        private static string DescribeProgramVersion(UInt32 programVersion)
        {
            uint kind, major, minor;
            kind = ((programVersion & 0xffff0000) >> 16);
            major = (programVersion & 0xf0) >> 4;
            minor = (programVersion & 0xf);
            string[] shaderKinds = "Pixel,Vertex,Geometry,Hull,Domain,Compute".Split(',');
            return shaderKinds[kind] + " " + major + "." + minor;
        }

        private static string DescribeProgramVersionShort(UInt32 programVersion)
        {
            uint kind, major, minor;
            kind = ((programVersion & 0xffff0000) >> 16);
            major = (programVersion & 0xf0) >> 4;
            minor = (programVersion & 0xf);
            string[] shaderKinds = "ps,vs,gs,hs,ds,cs".Split(',');
            return shaderKinds[kind] + "_" + major + "_" + minor;
        }

        private static TreeNode RangeNode(string text)
        {
            return new TreeNode(text);
        }

        private static TreeNode RangeNode(string text, int offset, int length)
        {
            TreeNode result = new TreeNode(text);
            result.Tag = new TreeNodeRange(offset, length);
            return result;
        }

        private static TreeNode RangeNodeASCII(byte[] bytes, string text, ref int offset, int charLength, out string value)
        {
            System.Diagnostics.Debug.Assert(offset % 8 == 0, "else NYI");
            int byteOffset = offset / 8;
            char[] valueChars = new char[charLength];
            for (int i = 0; i < charLength; ++i)
            {
                valueChars[i] = (char)bytes[byteOffset + i];
            }
            value = new string(valueChars);
            TreeNode result = RangeNode(text + ": '" + value + "'", offset, charLength * 8);
            offset += charLength * 8;
            return result;
        }

        private static uint ReadArrayBits(byte[] bytes, int offset, int length)
        {
            uint value = 0;
            int byteOffset = offset / 8;
            int bitOffset = offset % 8;
            for (int i = 0; i < length; ++i)
            {
                uint bit = (bytes[byteOffset] & (uint)(1 << bitOffset)) >> bitOffset;
                value |= bit << i;
                ++bitOffset;
                if (bitOffset == 8)
                {
                    byteOffset++;
                    bitOffset = 0;
                }
            }
            return value;
        }

        private static TreeNode RangeNodeBits(byte[] bytes, string text, ref int offset, int length, out uint value)
        {
            System.Diagnostics.Debug.Assert(length > 0 && length <= 32, "Cannot return zero or more than BitsInWord bits!");
            TreeNode result = RangeNode(text, offset, length);
            value = ReadArrayBits(bytes, offset, length);
            offset += length;
            return result;
        }

        private static TreeNode RangeNodeVBR(byte[] bytes, string text, ref int offset, int length, out uint value)
        {
            value = ReadArrayBits(bytes, offset, length);
            if ((value & (1 << (length - 1))) == 0)
            {
                TreeNode result = RangeNode(text + ": " + value, offset, length);
                offset += length;
                return result;
            }
            else
            {
                throw new NotImplementedException();
            }
        }

        private static TreeNode RangeNodeBytes(string text, ref int offset, int length)
        {
            TreeNode result = RangeNode(text, offset, length);
            offset += length;
            return result;
        }

        private static TreeNode RangeNodeUInt8(byte[] bytes, string text, ref int offset, out byte value)
        {
            System.Diagnostics.Debug.Assert(offset % 8 == 0, "else NYI");
            int byteOffset = offset / 8;
            value = bytes[byteOffset];
            TreeNode result = RangeNode(text + ": " + value, offset, 8);
            offset += 8;
            return result;
        }

        private static TreeNode RangeNodeUInt16(byte[] bytes, string text, ref int offset, out UInt16 value)
        {
            System.Diagnostics.Debug.Assert(offset % 8 == 0, "else NYI");
            int byteOffset = offset / 8;
            value = (ushort)((bytes[byteOffset]) + (bytes[byteOffset + 1] << 8));
            TreeNode result = RangeNode(text + ": " + value, offset, 16);
            offset += 16;
            return result;
        }

        private static TreeNode RangeNodeUInt32(byte[] bytes, string text, ref int offset, out UInt32 value)
        {
            System.Diagnostics.Debug.Assert(offset % 8 == 0, "else NYI");
            int byteOffset = offset / 8;
            value = (uint)((bytes[byteOffset]) | (bytes[byteOffset + 1] << 8) | (bytes[byteOffset + 2] << 16) | (bytes[byteOffset + 3] << 24));
            TreeNode result = RangeNode(text + ": " + value, offset, 32);
            offset += 32;
            return result;
        }

        #endregion Bitstream generation.

        private bool IsOKToOverwriteContent()
        {
            if (!this.DocModified)
                return true;
            return MessageBox.Show(this, "Are you sure you want to lose your changes?", "Changes Pending", MessageBoxButtons.YesNo) == DialogResult.Yes;
        }

        private void fileToolStripMenuItem_DropDownOpening(object sender, EventArgs e)
        {
            this.RefreshMRUMenu(this.mruManager, this.recentFilesToolStripMenuItem);
        }

        private void EditorForm_Load(object sender, EventArgs e)
        {
            this.mruManager = new MRUManager();
            this.mruManager.LoadFromFile();
            this.settingsManager = new SettingsManager();
            this.settingsManager.LoadFromFile();
            this.CheckSettingsForDxcLibrary();
        }

        private void RefreshMRUMenu(MRUManager mru, ToolStripMenuItem parent)
        {
            parent.DropDownItems.Clear();
            foreach (var item in mru.Paths)
            {
                EventHandler MRUHandler = (_sender, _args) =>
                {
                    if (!System.IO.File.Exists(item))
                    {
                        MessageBox.Show("File not found");
                        return;
                    }
                    this.HandleOpenUI(item);
                };
                parent.DropDownItems.Add(item, null, MRUHandler);
            }
        }

        private static IEnumerable<DiagnosticDetail> EnumerateDiagnosticDetails(DxcDiagnosticDisplayOptions options, IDxcTranslationUnit tu)
        {
            uint count = tu.GetNumDiagnostics();
            for (uint i = 0; i < count; ++i)
            {
                uint errorCode;
                uint errorLine;
                uint errorColumn;
                string errorFile;
                uint errorOffset;
                uint errorLength;
                string errorMessage;
                tu.GetDiagnosticDetails(i, options, out errorCode, out errorLine, out errorColumn, out errorFile, out errorOffset, out errorLength, out errorMessage);
                yield return new DiagnosticDetail()
                {
                    ErrorCode = (int)errorCode,
                    ErrorLine = (int)errorLine,
                    ErrorColumn = (int)errorColumn,
                    ErrorFile = errorFile,
                    ErrorOffset = (int)errorOffset,
                    ErrorLength = (int)errorLength,
                    ErrorMessage = errorMessage,
                };
            }
        }

        private static List<DiagnosticDetail> ListDiagnosticDetails(DxcDiagnosticDisplayOptions options, IDxcTranslationUnit tu)
        {
            return EnumerateDiagnosticDetails(options, tu).ToList();
        }

        private void RefreshDiagnosticDetails()
        {
            if (this.diagnosticDetailsGrid == null)
            {
                return;
            }

            IDxcTranslationUnit tu = this.GetTU();
            if (tu == null)
            {
                return;
            }

            DxcDiagnosticDisplayOptions options = this.isense.GetDefaultDiagnosticDisplayOptions();
            this.diagnosticDetails = ListDiagnosticDetails(options, tu);
            this.diagnosticDetailsGrid.DataSource = this.diagnosticDetails;
            this.diagnosticDetailsGrid.Columns["ErrorCode"].Visible = false;
            this.diagnosticDetailsGrid.Columns["ErrorLength"].Visible = false;
            this.diagnosticDetailsGrid.Columns["ErrorOffset"].Visible = false;
        }

        private void DiagnosticDetailsGridDoubleClick(object sender, EventArgs e)
        {
            if (this.diagnosticDetailsGrid.SelectedRows.Count == 0)
                return;
            DiagnosticDetail detail = this.diagnosticDetailsGrid.SelectedRows[0].DataBoundItem as DiagnosticDetail;
            if (detail == null)
                return;
            this.CodeBox.Select(detail.ErrorOffset, detail.ErrorLength);
            this.CodeBox.Select();
        }

        class PassArgumentControls
        {
            public PassArgInfo ArgInfo { get; set; }
            public Label PromptLabel { get; set; }
            public TextBox ValueControl { get; set; }
            public Label DescriptionLabel { get; set; }
            public IEnumerable<Control> Controls
            {
                get
                {
                    yield return PromptLabel;
                    yield return ValueControl;
                    yield return DescriptionLabel;
                }
            }
            public static PassArgumentControls FromArg(PassArgInfo arg)
            {
                PassArgumentControls result = new MainNs.EditorForm.PassArgumentControls()
                {
                    ArgInfo = arg,
                    PromptLabel = new Label()
                    {
                        Text = arg.Name,
                        UseMnemonic = true,
                    },
                    DescriptionLabel = new Label()
                    {
                        Text = arg.Description,
                        UseMnemonic = false,
                    },
                    ValueControl = new TextBox()
                    {
                    },
                };
                if (result.DescriptionLabel.Text == "None")
                {
                    result.DescriptionLabel.Visible = false;
                }
                return result;
            }
        }

        private int LayoutVertical(Control container, IEnumerable<Control> controls, int top, int pad)
        {
            int result = top;
            int controlWidth = container.ClientSize.Width - pad * 2;
            foreach (var c in controls)
            {
                if (!c.Visible)
                    continue;
                c.Top = result;
                c.Left = pad;
                c.Width = controlWidth;
                c.Anchor = AnchorStyles.Left | AnchorStyles.Top | AnchorStyles.Right;
                result += c.Height;
                result += pad;
                container.Controls.Add(c);
            }
            return result;
        }

        private void RemoveListBoxItems(ListBox listbox, int startIndex, int endIndexExclusive)
        {
            for (int i = endIndexExclusive - 1; i >= startIndex; --i)
            {
                listbox.Items.RemoveAt(i);
            }
        }

        private void PassPropertiesMenuItem_Click(object sender, EventArgs e)
        {
            ListBox lb = this.SelectedPassesBox;
            int selectedIndex = lb.SelectedIndex;
            PassInfoWithValues passInfoValues = lb.SelectedItem as PassInfoWithValues;
            if (passInfoValues == null)
            {
                return;
            }
            PassInfo passInfo = passInfoValues.PassInfo;
            string title = String.Format("{0} properties", passInfo.Name);
            if (passInfo.Args.Length == 0)
            {
                MessageBox.Show(this, "No properties available to set.", title);
                return;
            }
            using (Form form = new Form())
            {
                var argControls =
                    passInfo.Args.Select(p => PassArgumentControls.FromArg(p)).ToDictionary(c => c.ArgInfo);
                foreach (var val in passInfoValues.Values)
                    argControls[val.Arg].ValueControl.Text = val.Value;
                int lastTop = LayoutVertical(form, argControls.Values.SelectMany(c => c.Controls), 8, 8);
                form.ShowInTaskbar = false;
                form.MinimizeBox = false;
                form.MaximizeBox = false;
                form.Text = title;
                Button okButton = new Button()
                {
                    Anchor = AnchorStyles.Bottom | AnchorStyles.Right,
                    DialogResult = DialogResult.OK,
                    Text = "OK",
                    Top = lastTop,
                };
                okButton.Left = form.ClientSize.Width - 8 - okButton.Width;
                form.Controls.Add(okButton);
                form.AcceptButton = okButton;
                if (form.ShowDialog(this) == DialogResult.OK)
                {
                    passInfoValues.Values.Clear();

                    // Add options with values.
                    foreach (var argValues in argControls.Values)
                    {
                        if (String.IsNullOrEmpty(argValues.ValueControl.Text))
                            continue;
                        passInfoValues.Values.Add(new PassArgValueInfo()
                        {
                            Arg = argValues.ArgInfo,
                            Value = argValues.ValueControl.Text
                        });
                    }
                    lb.Items.RemoveAt(selectedIndex);
                    lb.Items.Insert(selectedIndex, passInfoValues);
                    lb.SelectedIndex = selectedIndex;
                }
            }
        }

        private void copyAllToolStripMenuItem_Click(object sender, EventArgs e)
        {
            StringBuilder sb = new StringBuilder();
            foreach (var o in this.SelectedPassesBox.Items)
                sb.AppendLine(o.ToString());
            Clipboard.SetText(sb.ToString());
        }

        private void renderToolStripMenuItem_Click(object sender, EventArgs e)
        {
            this.RenderLogBox.Clear();

            string payloadText = GetShaderOpPayload();

            if (this.settingsManager.ExternalRenderEnabled)
            {
                string path = System.IO.Path.Combine(System.IO.Path.GetTempPath(), "dndxc-ext-render.xml");
                try
                {
                    System.IO.File.WriteAllText(path, payloadText);
                }
                catch (Exception writeErr)
                {
                    HandleException(writeErr, "Unable to write render input to " + path);
                    return;
                }
                try
                {
                    string arguments = this.settingsManager.ExternalRenderCommand;
                    arguments = arguments.Replace("%in", path);
                    var process = System.Diagnostics.Process.Start("cmd.exe", "/c " + arguments);
                    if (process != null)
                    {
                        process.Dispose();
                    }
                }
                catch (Exception runErr)
                {
                    HandleException(runErr, "Unable to run external render command.");
                    return;
                }

                return;
            }

            try
            {
                this.hlslHost.EnsureActive();
            }
            catch (Exception startErr)
            {
                HandleException(startErr, "Unable to start HLSLHost.exe.");
                return;
            }

            try
            {
                SendHostMessageAndLogReply(HlslHost.HhMessageId.StartRendererMsgId);
                this.AnalysisTabControl.SelectedTab = RenderViewTabPage;
                this.hlslHost.SetParentHwnd(RenderViewTabPage.Handle);
                this.hlslHost.SendHostMessagePlay(payloadText);
                System.Windows.Forms.Timer t = new Timer();
                t.Interval = 1000;
                t.Tick += (_, __) =>
                {
                    t.Dispose();
                    if (!this.TopSplitContainer.Panel2Collapsed)
                    {
                        this.RenderLogBox.Font = this.CodeBox.Font;
                        this.DrainHostLog();
                    }
                };
                t.Start();
            }
            catch (Exception runError)
            {
                System.Diagnostics.Debug.WriteLine(runError);
                this.hlslHost.IsActive = false;
                this.HandleException(runError, "Unable to render");
            }
        }

        private string GetShaderOpPayload()
        {
            string fullText = this.CodeBox.Text;
            string xml = GetShaderOpXmlFragment(fullText);
            return HlslHost.GetShaderOpPayload(fullText, xml);
        }

        private static readonly string ShaderOpStartMarker = "#if SHADER_OP_XML";
        private static readonly string ShaderOpStopMarker = "#endif";

        private static string GetShaderOpXmlFragment(string text)
        {
            int start = text.IndexOf(ShaderOpStartMarker);
            if (start == -1)
                throw new InvalidOperationException("Cannot for '" + ShaderOpStartMarker + "' marker");
            start += ShaderOpStartMarker.Length;
            int end = text.IndexOf(ShaderOpStopMarker, start);
            if (end == -1)
                throw new InvalidOperationException("Cannot for '" + ShaderOpStopMarker + "' marker");
            return text.Substring(start, end - start).Trim();
        }

        private void SendHostMessageAndLogReply(HlslHost.HhMessageId kind)
        {
            this.hlslHost.SendHostMessage(kind);
            LogReply(this.hlslHost.GetReply());
        }

        private HlslHost.HhMessageReply LogReply(HlslHost.HhMessageReply reply)
        {
            if (reply == null)
                return null;
            string log = HlslHost.GetLogReplyText(reply);
            if (!String.IsNullOrWhiteSpace(log))
            {
                this.RenderLogBox.AppendText(log + "\r\n");
            }
            return reply;
        }

        private void DrainHostLog()
        {
            try
            {
                this.hlslHost.SendHostMessage(HlslHost.HhMessageId.ReadLogMsgId);
                for (;;)
                {
                    if (this.LogReply(this.hlslHost.GetReply()) == null)
                        return;
                }
            }
            catch (Exception hostErr)
            {
                this.TheStatusStripLabel.Text = "Unable to contact host.";
                this.RenderLogBox.AppendText(hostErr.Message);
            }
        }

        private void outputToolStripMenuItem_Click(object sender, EventArgs e)
        {
            this.TopSplitContainer.Panel2Collapsed =
                !this.TopSplitContainer.Panel2Collapsed;
            if (!this.hlslHost.IsActive)
                return;
            this.RenderLogBox.Font = this.CodeBox.Font;
            this.DrainHostLog();
        }

        private void EditorForm_FormClosing(object sender, FormClosingEventArgs e)
        {
            // Consider prompting to save.
        }

        private void EditorForm_FormClosed(object sender, FormClosedEventArgs e)
        {
            this.hlslHost.IsActive = false;
        }

        private void FontGrowToolStripMenuItem_Click(object sender, EventArgs e)
        {
            Control target = this.DeepActiveControl;
            if (target == null) return;
            target.Font = new Font(target.Font.FontFamily, target.Font.Size * 1.1f);
        }

        private void FontShrinkToolStripMenuItem_Click(object sender, EventArgs e)
        {
            Control target = this.DeepActiveControl;
            if (target == null) return;
            target.Font = new Font(target.Font.FontFamily, target.Font.Size / 1.1f);
        }

        private void optionsToolStripMenuItem_Click(object sender, EventArgs e)
        {
            Form form = new Form();
            PropertyGrid grid = new PropertyGrid();
            grid.SelectedObject = this.settingsManager;
            grid.Dock = DockStyle.Fill;
            form.Controls.Add(grid);
            form.FormClosing += (_, __) =>
            {
                this.settingsManager.SaveToFile();
                this.CheckSettingsForDxcLibrary();
            };
            form.ShowDialog(this);
        }

        private void CheckSettingsForDxcLibrary()
        {
            try
            {
                if (String.IsNullOrWhiteSpace(this.settingsManager.ExternalLib))
                {
                    HlslDxcLib.DxcCreateInstanceFn = DefaultDxcLib.GetDxcCreateInstanceFn();
                }
                else
                {
                    HlslDxcLib.DxcCreateInstanceFn = HlslDxcLib.LoadDxcCreateInstance(
                        this.settingsManager.ExternalLib,
                        this.settingsManager.ExternalFunction);
                }
            }
            catch (Exception e)
            {
                HandleException(e);
            }
        }

        private void colorToolStripMenuItem_Click(object sender, EventArgs e)
        {
            this.ColorMenuItem.Checked = !this.ColorMenuItem.Checked;
            CodeBox_SelectionChanged(sender, e);
        }

        private void rewriterToolStripMenuItem_Click(object sender, EventArgs e)
        {
            IDxcRewriter rewriter = HlslDxcLib.CreateDxcRewriter();
            IDxcBlobEncoding code = CreateBlobForCodeText();
            IDxcRewriteResult rewriterResult = rewriter.RewriteUnchangedWithInclude(code, "input.hlsl", null, 0, library.CreateIncludeHandler(), 0);
            IDxcBlobEncoding rewriteBlob = rewriterResult.GetRewrite();
            string rewriteText = GetStringFromBlob(rewriteBlob);
            RewriterOutputTextBox.Text = rewriteText;
            AnalysisTabControl.SelectTab(RewriterOutputTabPage);
        }

        private void rewriteNobodyToolStripMenuItem_Click(object sender, EventArgs e)
        {
            IDxcRewriter rewriter = HlslDxcLib.CreateDxcRewriter();
            IDxcBlobEncoding code = CreateBlobForCodeText();
            IDxcRewriteResult rewriterResult = rewriter.RewriteUnchangedWithInclude(code, "input.hlsl", null, 0, library.CreateIncludeHandler(), 1);
            IDxcBlobEncoding rewriteBlob = rewriterResult.GetRewrite();
            string rewriteText = GetStringFromBlob(rewriteBlob);
            RewriterOutputTextBox.Text = rewriteText;
            AnalysisTabControl.SelectTab(RewriterOutputTabPage);
        }

        private IEnumerable<string> EnumerateDiaCandidates()
        {
            string progPath = Environment.GetFolderPath(Environment.SpecialFolder.ProgramFilesX86);
            yield return System.IO.Path.Combine(progPath, @"Microsoft Visual Studio\2017\Community\DIA SDK\bin\amd64\msdia140.dll");
            yield return System.IO.Path.Combine(progPath, @"Microsoft Visual Studio\2017\Community\Common7\IDE\msdia140.dll");
            yield return System.IO.Path.Combine(progPath, @"Microsoft Visual Studio\2017\Community\Common7\IDE\msdia120.dll");
        }

        private string SuggestDiaRegistration()
        {
            foreach (string path in EnumerateDiaCandidates())
            {
                if (System.IO.File.Exists(path))
                {
                    return "Consider registering with this command:\r\n" +
                        "regsvr32 \"" + path + "\"";
                }
            }
            return null;
        }

        private void debugInformationToolStripMenuItem_Click(object sender, EventArgs e)
        {
            if (this.SelectedShaderBlob == null)
            {
                MessageBox.Show("Compile or load a shader to view debug information.");
                return;
            }

            this.ClearDiaUnimplementedFlags();
            IDxcBlob debugInfoBlob;
            dia2.IDiaDataSource dataSource;
            uint dfcc;
            IDxcContainerReflection r = HlslDxcLib.CreateDxcContainerReflection();
            r.Load(this.SelectedShaderBlob);
            if (IsDxilTarget(this.selectedShaderBlobTarget))
            {
                dataSource = HlslDxcLib.CreateDxcDiaDataSource();
                dfcc = DFCC_ILDB;
            }
            else
            {
                try
                {
                    dataSource = new dia2.DiaDataSource() as dia2.IDiaDataSource;
                }
                catch (System.Runtime.InteropServices.COMException ce)
                {
                    const int REGDB_E_CLASSNOTREG = -2147221164;
                    if (ce.HResult == REGDB_E_CLASSNOTREG)
                    {
                        string suggestion = SuggestDiaRegistration();
                        string message = "Unable to create dia object.";
                        if (suggestion != null) message += "\r\n" + suggestion;
                        MessageBox.Show(this, message);
                    }
                    else
                    {
                        HandleException(ce);
                    }
                    return;
                }
                dfcc = DFCC_SPDB;
            }
            uint index;
            int hr = r.FindFirstPartKind(dfcc, out index);
            if (hr < 0)
            {
                MessageBox.Show("Debug information not found in container.");
                return;
            }
            debugInfoBlob = r.GetPartContent(index);
            try
            {
                dataSource.loadDataFromIStream(Library.CreateStreamFromBlobReadOnly(debugInfoBlob));
                string s = dataSource.get_lastError();
                if (!String.IsNullOrEmpty(s))
                {
                    MessageBox.Show("Failure to load stream: " + s);
                    return;
                }
            }
            catch (Exception ex)
            {
                HandleException(ex);
                return;
            }
            var session = dataSource.openSession();
            var tables = session.getEnumTables();
            uint count = tables.get_Count();
            StringBuilder output = new StringBuilder();
            output.AppendLine("* Tables");
            bool isFirstTable = true;
            for (uint i = 0; i < count; ++i)
            {
                var table = tables.Item(i);
                if (isFirstTable) isFirstTable = false; else output.AppendLine();
                output.AppendLine("** " + table.get_name());
                int itemCount = table.get_Count();
                output.AppendLine("Record count: " + itemCount);
                for (int itemIdx = 0; itemIdx < itemCount; itemIdx++)
                {
                    object o = table.Item((uint)itemIdx);
                    if (TryDumpDiaObject(o as dia2.IDiaSymbol, output)) continue;
                    if (TryDumpDiaObject(o as dia2.IDiaSourceFile, output)) continue;
                    if (TryDumpDiaObject(o as dia2.IDiaLineNumber, output)) continue;
                    if (TryDumpDiaObject(o as dia2.IDiaInjectedSource, output)) continue;
                    if (TryDumpDiaObject(o as dia2.IDiaSectionContrib, output)) continue;
                    if (TryDumpDiaObject(o as dia2.IDiaSegment, output)) continue;
                }
            }

            if (debugInfoTabPage == null)
            {
                this.debugInfoTabPage = new TabPage("Debug Info");
                this.debugInfoControl = new RichTextBox()
                {
                    Dock = DockStyle.Fill,
                    Font = this.CodeBox.Font,
                    ReadOnly = true
                };
                this.AnalysisTabControl.TabPages.Add(this.debugInfoTabPage);
                this.debugInfoTabPage.Controls.Add(this.debugInfoControl);
            }
            this.debugInfoControl.Text = output.ToString();
            this.AnalysisTabControl.SelectedTab = this.debugInfoTabPage;
        }

        private bool TryDumpDiaObject<TIface>(TIface o, StringBuilder sb)
        {
            if (o == null) return false;
            DumpDiaObject(o, sb);
            return true;
        }

        private void ClearDiaUnimplementedFlags()
        {
            foreach (var item in TypeWriters)
                foreach (var writer in item.Value)
                    writer.Unimplemented = false;
        }

        private void DumpDiaObject<TIface>(TIface o, StringBuilder sb)
        {
            Type type = typeof(TIface);
            bool hasLine = false;
            List<DiaTypePropertyWriter> writers;
            if (SymTagEnumValues == null)
            {
                SymTagEnumValues = Enum.GetNames(typeof(dia2.SymTagEnum));
            }
            if (!TypeWriters.TryGetValue(type, out writers))
            {
                writers = new List<DiaTypePropertyWriter>();
                foreach (System.Reflection.MethodInfo mi in type.GetMethods())
                {
                    Func<string, object, StringBuilder, bool> writer;
                    if (mi.GetParameters().Length > 0)
                        continue;
                    string propertyName = mi.Name;
                    if (!propertyName.StartsWith("get_")) continue;
                    propertyName = propertyName.Substring(4);
                    Type returnType = mi.ReturnType;
                    if (returnType == typeof(string))
                        writer = WriteDiaValueString;
                    else if (returnType == typeof(uint) && propertyName == "symTag")
                        writer = WriteDiaValueSymTag;
                    else if (returnType == typeof(uint))
                        writer = WriteDiaValueUInt32;
                    else if (returnType == typeof(UInt64))
                        writer = WriteDiaValueUInt64;
                    else if (returnType == typeof(bool))
                        writer = WriteDiaValueBool;
                    else
                        writer = WriteDiaValueAny;
                    writers.Add(new DiaTypePropertyWriter()
                    {
                        MI = mi,
                        PropertyName = propertyName,
                        Writer = writer
                    });
                }
                TypeWriters[type] = writers;
            }
            foreach (var writer in writers)
            {
                if (writer.Write(o, sb))
                    hasLine = true;
            }
            if (hasLine) sb.AppendLine();
        }

        private static Dictionary<Type, List<DiaTypePropertyWriter>> TypeWriters = new Dictionary<Type, List<DiaTypePropertyWriter>>();
        internal static string[] SymTagEnumValues;

        class DiaTypePropertyWriter
        {
            public static object[] EmptyParams = new object[0];
            public bool Unimplemented;
            public System.Reflection.MethodInfo MI;
            public string PropertyName;
            public Func<string, object, StringBuilder, bool> Writer;
            internal bool Write(object instance, StringBuilder sb)
            {
                if (Unimplemented)
                    return false;
                try
                {
                    object value = MI.Invoke(instance, EmptyParams);
                    return Writer(PropertyName, value, sb);
                }
                catch (System.Reflection.TargetInvocationException tie)
                {
                    if (tie.InnerException is NotImplementedException)
                    {
                        this.Unimplemented = true;
                    }
                    return false;
                }
                catch (System.Runtime.InteropServices.COMException)
                {
                    return false;
                }
            }
        }

        private static bool WriteDiaValueAny(string propertyName, object propertyValue, StringBuilder sb)
        {
            if (null == propertyValue) return false;
            if (System.Runtime.InteropServices.Marshal.IsComObject(propertyValue)) return false;
            sb.Append(propertyName);
            sb.Append(": ");
            sb.Append(Convert.ToString(propertyValue));
            sb.AppendLine();
            return true;
        }

        private static bool WriteDiaValueSymTag(string propertyName, object propertyValueObj, StringBuilder sb)
        {
            uint tag = (uint)propertyValueObj;
            sb.Append(propertyName);
            sb.Append(": ");
            sb.AppendLine(SymTagEnumValues[tag]);
            return true;
        }

        private static bool WriteDiaValueBool(string propertyName, object propertyValueObj, StringBuilder sb)
        {
            bool propertyValue = (bool)propertyValueObj;
            if (false == propertyValue) return false;
            sb.Append(propertyName);
            sb.Append(": ");
            sb.Append(propertyValue);
            sb.AppendLine();
            return true;
        }

        private static bool WriteDiaValueUInt32(string propertyName, object propertyValueObj, StringBuilder sb)
        {
            uint propertyValue = (uint)propertyValueObj;
            if (0 == propertyValue) return false;
            sb.Append(propertyName);
            sb.Append(": ");
            sb.Append(propertyValue);
            sb.AppendLine();
            return true;
        }

        private static bool WriteDiaValueUInt64(string propertyName, object propertyValueObj, StringBuilder sb)
        {
            UInt64 propertyValue = (UInt64)propertyValueObj;
            if (0 == propertyValue) return false;
            sb.Append(propertyName);
            sb.Append(": ");
            sb.Append(propertyValue);
            sb.AppendLine();
            return true;
        }

        private static bool WriteDiaValueString(string propertyName, object propertyValueObj, StringBuilder sb)
        {
            string propertyValue = (string)propertyValueObj;
            if (String.IsNullOrEmpty(propertyValue)) return false;
            sb.Append(propertyName);
            sb.Append(": ");
            sb.AppendLine(propertyValue);
            return true;
        }

        private void PastePassesMenuItem_Click(object sender, EventArgs e)
        {
            if (!Clipboard.ContainsText())
            {
                MessageBox.Show(this, "The clipboard does not contain pass information.", "Unable to paste passes");
                return;
            }
            string passes = Clipboard.GetText(TextDataFormat.UnicodeText);
            try
            {
                passes = passes.Replace("\r\n", "\n");
                AddSelectedPassesFromText(passes, false);
            }
            catch (Exception ex)
            {
                this.HandleException(ex);
            }
        }

        private void DeleteAllPassesMenuItem_Click(object sender, EventArgs e)
        {
            this.SelectedPassesBox.Items.Clear();
        }

        public class AssembleResult
        {
            public IDxcBlob Blob { get; set; }
            public string ResultText { get; set; }
            public bool Succeeded { get; set; }
        }

        public class HighLevelCompileResult
        {
            public IDxcBlob Blob { get; set; }
            public string ResultText { get; set; }
        }

        public class OptimizeResult
        {
            public IDxcBlob Module { get; set; }
            public string ResultText { get; set; }
            public bool Succeeded { get; set; }
        }

        public static AssembleResult RunAssembly(IDxcLibrary library, IDxcBlob source)
        {
            IDxcBlob resultBlob = null;
            string resultText = "";
            bool succeeded;
            var assembler = HlslDxcLib.CreateDxcAssembler();
            var result = assembler.AssembleToContainer(source);
            if (result.GetStatus() == 0)
            {
                resultBlob = result.GetResult();
                succeeded = true;
            }
            else
            {
                resultText = GetStringFromBlob(library, result.GetErrors());
                succeeded = false;
            }
            return new AssembleResult() { Blob = resultBlob, ResultText = resultText, Succeeded = succeeded };
        }

        public HighLevelCompileResult RunHighLevelCompile()
        {
            IDxcCompiler compiler = HlslDxcLib.CreateDxcCompiler();
            string fileName = "hlsl.hlsl";
            HlslFileVariables fileVars = GetFileVars();
            List<string> args = new List<string>();
            args.Add("-fcgl");
            args.AddRange(tbOptions.Text.Split());
            string resultText = "";
            IDxcBlob source = null;
            {
                try
                {
                    var result = compiler.Compile(this.CreateBlobForCodeText(), fileName, fileVars.Entry, fileVars.Target, args.ToArray(), args.Count, null, 0, library.CreateIncludeHandler());
                    if (result.GetStatus() == 0)
                    {
                        source = result.GetResult();
                    }
                    else
                    {
                        resultText = GetStringFromBlob(result.GetErrors());
                    }
                }
                catch (System.ArgumentException e)
                {
                    MessageBox.Show(this, $"{e.Message}.", "Invalid form entry");
                }
            }
            return new HighLevelCompileResult() { Blob = source, ResultText = resultText };
        }

        public static OptimizeResult RunOptimize(IDxcLibrary library, string[] options, string source)
        {
            return RunOptimize(library, options, CreateBlobForText(library, source));
        }

        public static OptimizeResult RunOptimize(IDxcLibrary library, string[] options, IDxcBlob source)
        {
            IDxcOptimizer opt = HlslDxcLib.CreateDxcOptimizer();
            IDxcBlob module = null;
            string resultText = "";
            IDxcBlobEncoding text;
            bool succeeded = true;
            try
            {
                opt.RunOptimizer(source, options, options.Length, out module, out text);
                resultText = GetStringFromBlob(library, text);
            }
            catch (Exception optException)
            {
                succeeded = false;
                resultText = "Failed to run optimizer: " + optException.Message;
            }
            return new OptimizeResult() { Module = module, ResultText = resultText, Succeeded = succeeded };
        }

        private void InteractiveEditorButton_Click(object sender, EventArgs e)
        {
            // Do a high-level only compile.
            HighLevelCompileResult compile = RunHighLevelCompile();
            if (compile.Blob == null)
            {
                MessageBox.Show("Failed to compile: " + compile.ResultText);
                return;
            }

            string[] options = CreatePassOptions(false, true);
            OptimizeResult opt = RunOptimize(this.Library, options, compile.Blob);
            if (!opt.Succeeded)
            {
                MessageBox.Show("Failed to optimize: " + opt.ResultText);
                return;
            }

            OptEditorForm form = new OptEditorForm();
            form.CodeFont = this.CodeBox.Font;
            form.HighLevelSource = compile.Blob;
            form.Library = this.Library;
            form.Sections = TextSection.EnumerateSections(new string[] { "MODULE-PRINT", "Phase:" }, opt.ResultText).ToArray();
            form.StartPosition = FormStartPosition.CenterParent;
            form.Show(this);
        }

        private void CodeBox_HelpRequested(object sender, HelpEventArgs hlpevent)
        {
            RichTextBox rtb = this.CodeBox;
            SelectionExpandResult expand = SelectionExpandResult.Expand(rtb);
            if (expand.IsEmpty)
                return;
            string readmeText;
            using (System.IO.StreamReader reader =
                new System.IO.StreamReader(System.Reflection.Assembly.GetEntryAssembly().GetManifestResourceStream("MainNs.README.md")))
            {
                readmeText = reader.ReadToEnd();
            }
            this.HelpControl.Text = readmeText;
            (this.HelpTabPage.Parent as TabControl).SelectedTab = this.HelpTabPage;
            int pos = readmeText.IndexOf(expand.Token, StringComparison.InvariantCultureIgnoreCase);
            if (pos >= 0)
            {
                this.HelpControl.Select(pos, 0);
                this.HelpControl.ScrollToCaret();
            }
        }
    }

    public static class RichTextBoxExt
    {
        public static void AppendLine(this RichTextBox rtb, string line, Color c)
        {
            rtb.SelectionBackColor = c;
            rtb.AppendText(line);
            rtb.AppendText("\r\n");
        }

        public static void AppendLines(this RichTextBox rtb, string prefix, string[] lines, int start, int length, Color c)
        {
            for (int i = start; i < (start + length); ++i)
            {
                rtb.AppendLine(prefix + lines[i], c);
            }
        }

        public static IEnumerable<string> ToStringEnumerable(this ListBox.ObjectCollection collection)
        {
            foreach (var item in collection)
                yield return item.ToString();
        }
    }
}
