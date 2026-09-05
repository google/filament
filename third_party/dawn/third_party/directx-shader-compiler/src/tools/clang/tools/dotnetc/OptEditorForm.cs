using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using DotNetDxc;

namespace MainNs
{
    public partial class OptEditorForm : Form
    {
        private TextSection[] sections;

        public OptEditorForm()
        {
            InitializeComponent();
        }

        public Font CodeFont
        {
            get { return this.CodeBox.Font; }
            set
            {
                this.CodeBox.Font = value;
                this.LogBox.Font = value;
            }
        }

        public TextSection[] Sections
        {
            get
            {
                return sections;
            }
            set
            {
                this.sections = value;
                PassesListBox.Items.AddRange(value);
            }
        }

        public IDxcLibrary Library { get; set; }
        public IDxcBlob HighLevelSource { get; set; }

        private void UpdateCodeBox()
        {
            CodeBox.Clear();
            int index = PassesListBox.SelectedIndex;
            if (index == -1) return;
            TextSection section = (TextSection)PassesListBox.SelectedItem;
            TextSection prior = index == 0 ? null : PassesListBox.Items[index - 1] as TextSection;
            if (prior == null || section.Text == prior.Text || RightButton.Checked)
                CodeBox.Text = section.Text;
            else if (LeftButton.Checked)
                CodeBox.Text = (prior == null) ? "(no prior text)" : prior.Text;
            else
                TextSection.RunDiff(prior, section, CodeBox);
            CodeBox.Modified = false;
            InvalidateApplyChanges();
        }

        private void LeftButton_CheckedChanged(object sender, EventArgs e)
        {
            UpdateCodeBox();
        }

        private void ApplyChangesButton_Click(object sender, EventArgs e)
        {
            // Turn the text into the expected encoding.
            IDxcBlobEncoding sourceBlob = EditorForm.CreateBlobForText(this.Library, this.CodeBox.Text);
            sourceBlob = this.Library.GetBlobAstUf8(sourceBlob);
            IDxcBlob bitcodeBlob = sourceBlob;

            List<string> passes = new List<string>();
            passes.Add("hlsl-passes-resume");
            for (int i = PassesListBox.SelectedIndex; i < PassesListBox.Items.Count; ++i)
            {
                passes.Add(((TextSection)PassesListBox.Items[i]).Title);
            }
            string[] options = EditorForm.CreatePassOptions(passes, false, true);
            EditorForm.OptimizeResult opt = EditorForm.RunOptimize(this.Library, options, bitcodeBlob);
            if (!opt.Succeeded)
            {
                MessageBox.Show("Failed to optimize: " + opt.ResultText);
                return;
            }

            OptEditorForm form = new OptEditorForm();
            form.CodeFont = this.CodeBox.Font;
            form.Library = this.Library;
            form.HighLevelSource = this.HighLevelSource;
            form.Sections = TextSection.EnumerateSections(new string[] { "MODULE-PRINT", "Phase:" }, opt.ResultText).ToArray();
            form.StartPosition = FormStartPosition.CenterParent;
            form.Show(this);
        }

        private void OptEditorForm_Load(object sender, EventArgs e)
        {
            RichTextBox rtb = CodeBox;
            var helper = new EditorForm.LogContextMenuHelper(rtb);
            rtb.ContextMenu = new ContextMenu(
                new MenuItem[]
                {
                    new MenuItem("Show Graph", helper.ShowGraphClick)
                });
        }

        private void CodeBox_TextChanged(object sender, EventArgs e)
        {
            InvalidateApplyChanges();
        }

        private void InvalidateApplyChanges()
        {
            this.ApplyChangesButton.Enabled = this.CodeBox.Modified && RightButton.Checked;
        }

        private void PassesListBox_SelectedIndexChanged(object sender, EventArgs e)
        {
            UpdateCodeBox();
        }

        private void CodeBox_SelectionChanged(object sender, EventArgs e)
        {
            EditorForm.HandleCodeSelectionChanged(this.CodeBox, null);
        }

        private void CopyContainerButton_Click(object sender, EventArgs e)
        {
            // The intent is to copy compiled code (possibly customized) into the
            // clipboard so it can be pasted into an XML file to be run interactively.

            var text = this.CodeBox.Text;
            var source = EditorForm.CreateBlobForText(this.Library, text);
            var assembler = HlslDxcLib.CreateDxcAssembler();
            var assembleResult = assembler.AssembleToContainer(source);
            if (assembleResult.GetStatus() < 0)
            {
                var errors = assembleResult.GetErrors();
                MessageBox.Show(EditorForm.GetStringFromBlob(this.Library, errors));
                return;
            }
            var container = assembleResult.GetResult();

            // Now copy that to the clipboard.
            var bytes = ContainerData.BlobToBytes(container);
            var stream = new System.IO.MemoryStream(bytes);
            var dataObj = new DataObject();
            dataObj.SetData(ContainerData.DataFormat.Name, stream);
            dataObj.SetText(text);
            Clipboard.SetDataObject(dataObj, true);
        }

        private void btnSaveAll_Click(object sender, EventArgs e)
        {
            saveFileDialog1.ShowDialog();
            string fileName = saveFileDialog1.FileName;
            for (int i = 0; i < sections.Length; i++)
            {
                TextSection Section = sections[i];
                string fullName = string.Format("{0}_{1}_{2}.ll", fileName, i, Section.Title);
                System.IO.File.WriteAllLines(fullName, Section.Lines);
            }
        }

        private void btnViewCFGOnly_Click(object sender, EventArgs e)
        {
            if (PassesListBox.SelectedIndex == -1)
            {
                MessageBox.Show("Select a pass first");
                return;
            }
            TextSection section = (TextSection)PassesListBox.SelectedItem;

            var source = EditorForm.CreateBlobForText(this.Library, section.Text);
            source = this.Library.GetBlobAstUf8(source);

            string[] options = new string[1];
            options[0] = "-view-cfg-only";
            EditorForm.OptimizeResult opt = EditorForm.RunOptimize(this.Library, options, source);
            if (!opt.Succeeded)
            {
                MessageBox.Show("Failed to optimize: " + opt.ResultText);
                return;
            }
            const string dotStart = "digraph";
            string dotText = opt.ResultText.Substring(opt.ResultText.IndexOf(dotStart));
            while (dotText.LastIndexOf(dotStart) != -1)
            {
                string dot = dotText.Substring(dotText.LastIndexOf(dotStart));
                dot = dot.Substring(0, dot.LastIndexOf("}") + 1);
                dot = dot.Replace("\u0001??", "");
                dot = dot.Replace("\u0001?", "");
                EditorForm.LogContextMenuHelper.ShowDot(dot);
                dotText = dotText.Substring(0, dotText.LastIndexOf(dotStart));
            }

        }
    }

    public class TextSection
    {
        private string[] lines;
        private int[] lineHashes;
        public string Title;
        public string Text;
        public bool HasChange;

        public string[] Lines
        {
            get
            {
                if (lines == null)
                    lines = Text.Split(new char[] { '\n' });
                return lines;
            }
        }
        public int[] LineHashes
        {
            get
            {
                if (lineHashes == null)
                    lineHashes = lines.Select(l => l.GetHashCode()).ToArray();
                return lineHashes;
            }
        }
        public override string ToString()
        {
            return HasChange ? "* " + Title : Title;
        }

        private static bool ClosestMatch(string text, ref int start, string[] separators, out string separator)
        {
            int closest = -1;
            separator = null;
            for (int i = 0; i < separators.Length; ++i)
            {
                int idx = text.IndexOf(separators[i], start);
                if (idx >= 0 && (closest < 0 || idx < closest))
                {
                    closest = idx;
                    separator = separators[i];
                }
            }
            start = closest;
            return closest >= 0;
        }

        public static IEnumerable<TextSection> EnumerateSections(string[] separators, string text)
        {
            string prior = null;
            string separator;
            int idx = 0;
            while (idx >= 0 && ClosestMatch(text, ref idx, separators, out separator))
            {
                int lineEnd = text.IndexOf('\n', idx);
                if (lineEnd < 0) break;
                string title = text.Substring(idx + separator.Length, lineEnd - (idx + separator.Length));
                title = title.Trim();
                int next = lineEnd;
                if (!ClosestMatch(text, ref next, separators, out separator))
                    next = -1;
                string sectionText = (next < 0) ? text.Substring(lineEnd + 1) : text.Substring(lineEnd + 1, next - (lineEnd + 1));
                sectionText = sectionText.Trim() + "\n";
                bool hasChange = sectionText != prior;
                yield return new TextSection { HasChange = hasChange, Title = title, Text = hasChange ? sectionText : prior };
                idx = next;
                prior = sectionText;
            }
        }

        public static void RunDiff(TextSection oldText, TextSection newText, RichTextBox rtb)
        {
            // Longest common subsequence, simple edition. If/when something faster is needed,
            // should probably take a dependency on a proper diff package. Other than shorter
            // comparison windows, other things to look for are avoiding creating strings here,
            // working on the RichTextBox buffer directly for color, and unique'ing lines.
            string[] oldLines = oldText.Lines;
            string[] newLines = newText.Lines;
            // Reduce strings to hashes.
            int[] oldHashes = oldText.LineHashes;
            int[] newHashes = newText.LineHashes;
            // Reduce by trimming prefix and suffix.
            int diffStart = 0;
            while (diffStart < oldHashes.Length && diffStart < newHashes.Length && oldHashes[diffStart] == newHashes[diffStart])
                diffStart++;
            int newDiffEndExc = newLines.Length, oldDiffEndExc = oldLines.Length;
            while (newDiffEndExc > diffStart && oldDiffEndExc > diffStart)
            {
                if (oldHashes[oldDiffEndExc - 1] == newHashes[newDiffEndExc - 1])
                {
                    oldDiffEndExc--;
                    newDiffEndExc--;
                }
                else
                    break;
            }
            int suffixLength = (newLines.Length - newDiffEndExc);
            // Build LCS table.
            int oldLen = oldDiffEndExc - diffStart, newLen = newDiffEndExc - diffStart;
            int[,] lcs = new int[oldLen + 1, newLen + 1]; // already zero-initialized
            for (int i = 0; i < oldLen; i++)
                for (int j = 0; j < newLen; j++)
                    if (oldHashes[i + diffStart] == newHashes[j + diffStart])
                        lcs[i + 1, j + 1] = lcs[i, j] + 1;
                    else
                        lcs[i + 1, j + 1] = Math.Max(lcs[i, j + 1], lcs[i + 1, j]);
            // Print the diff - common prefix, backtracked diff and common suffix.
            rtb.AppendLines("  ", newLines, 0, diffStart, Color.White);
            {
                int i = oldLen, j = newLen;
                Stack<string> o = new Stack<string>();
                for (;;)
                {
                    if (i > 0 && j > 0 && oldHashes[diffStart + i - 1] == newHashes[diffStart + j - 1])
                    {
                        o.Push("  " + oldLines[diffStart + i - 1]);
                        i--;
                        j--;
                    }
                    else if (j > 0 && (i == 0 || lcs[i, j - 1] >= lcs[i - 1, j]))
                    {
                        o.Push("+ " + newLines[diffStart + j - 1]);
                        j--;
                    }
                    else if (i > 0 && (j == 0 || lcs[i, j - 1] < lcs[i - 1, j]))
                    {
                        o.Push("- " + oldLines[diffStart + i - 1]);
                        i--;
                    }
                    else
                    {
                        break;
                    }
                }
                while (o.Count != 0)
                {
                    string line = o.Pop();
                    Color c = (line[0] == ' ') ? Color.White :
                        ((line[0] == '+') ? Color.Yellow : Color.Red);
                    rtb.AppendLine(line, c);
                }
            }
            rtb.AppendLines("  ", newLines, newDiffEndExc, (newLines.Length - newDiffEndExc), Color.White);
        }
    }
}
