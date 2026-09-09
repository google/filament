namespace MainNs
{
    partial class OptEditorForm
    {
        /// <summary>
        /// Required designer variable.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        /// Clean up any resources being used.
        /// </summary>
        /// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Windows Form Designer generated code

        /// <summary>
        /// Required method for Designer support - do not modify
        /// the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            this.TopContainer = new System.Windows.Forms.SplitContainer();
            this.PassesListBox = new System.Windows.Forms.ListBox();
            this.WorkContainer = new System.Windows.Forms.SplitContainer();
            this.CodeBox = new System.Windows.Forms.RichTextBox();
            this.flowLayoutPanel1 = new System.Windows.Forms.FlowLayoutPanel();
            this.LeftButton = new System.Windows.Forms.RadioButton();
            this.DiffButton = new System.Windows.Forms.RadioButton();
            this.RightButton = new System.Windows.Forms.RadioButton();
            this.ApplyChangesButton = new System.Windows.Forms.Button();
            this.CopyContainerButton = new System.Windows.Forms.Button();
            this.btnSaveAll = new System.Windows.Forms.Button();
            this.btnViewCFGOnly = new System.Windows.Forms.Button();
            this.LogBox = new System.Windows.Forms.RichTextBox();
            this.saveFileDialog1 = new System.Windows.Forms.SaveFileDialog();
            ((System.ComponentModel.ISupportInitialize)(this.TopContainer)).BeginInit();
            this.TopContainer.Panel1.SuspendLayout();
            this.TopContainer.Panel2.SuspendLayout();
            this.TopContainer.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)(this.WorkContainer)).BeginInit();
            this.WorkContainer.Panel1.SuspendLayout();
            this.WorkContainer.Panel2.SuspendLayout();
            this.WorkContainer.SuspendLayout();
            this.flowLayoutPanel1.SuspendLayout();
            this.SuspendLayout();
            // 
            // TopContainer
            // 
            this.TopContainer.Dock = System.Windows.Forms.DockStyle.Fill;
            this.TopContainer.Location = new System.Drawing.Point(0, 0);
            this.TopContainer.Name = "TopContainer";
            // 
            // TopContainer.Panel1
            // 
            this.TopContainer.Panel1.Controls.Add(this.PassesListBox);
            // 
            // TopContainer.Panel2
            // 
            this.TopContainer.Panel2.Controls.Add(this.WorkContainer);
            this.TopContainer.Size = new System.Drawing.Size(1497, 901);
            this.TopContainer.SplitterDistance = 498;
            this.TopContainer.TabIndex = 0;
            // 
            // PassesListBox
            // 
            this.PassesListBox.Dock = System.Windows.Forms.DockStyle.Fill;
            this.PassesListBox.FormattingEnabled = true;
            this.PassesListBox.Location = new System.Drawing.Point(0, 0);
            this.PassesListBox.Name = "PassesListBox";
            this.PassesListBox.Size = new System.Drawing.Size(498, 901);
            this.PassesListBox.TabIndex = 0;
            this.PassesListBox.SelectedIndexChanged += new System.EventHandler(this.PassesListBox_SelectedIndexChanged);
            // 
            // WorkContainer
            // 
            this.WorkContainer.Dock = System.Windows.Forms.DockStyle.Fill;
            this.WorkContainer.FixedPanel = System.Windows.Forms.FixedPanel.Panel2;
            this.WorkContainer.Location = new System.Drawing.Point(0, 0);
            this.WorkContainer.Name = "WorkContainer";
            this.WorkContainer.Orientation = System.Windows.Forms.Orientation.Horizontal;
            // 
            // WorkContainer.Panel1
            // 
            this.WorkContainer.Panel1.Controls.Add(this.CodeBox);
            this.WorkContainer.Panel1.Controls.Add(this.flowLayoutPanel1);
            // 
            // WorkContainer.Panel2
            // 
            this.WorkContainer.Panel2.Controls.Add(this.LogBox);
            this.WorkContainer.Size = new System.Drawing.Size(995, 901);
            this.WorkContainer.SplitterDistance = 824;
            this.WorkContainer.TabIndex = 0;
            // 
            // CodeBox
            // 
            this.CodeBox.Dock = System.Windows.Forms.DockStyle.Fill;
            this.CodeBox.Location = new System.Drawing.Point(0, 32);
            this.CodeBox.Name = "CodeBox";
            this.CodeBox.Size = new System.Drawing.Size(995, 792);
            this.CodeBox.TabIndex = 1;
            this.CodeBox.Text = "";
            this.CodeBox.SelectionChanged += new System.EventHandler(this.CodeBox_SelectionChanged);
            this.CodeBox.TextChanged += new System.EventHandler(this.CodeBox_TextChanged);
            // 
            // flowLayoutPanel1
            // 
            this.flowLayoutPanel1.AutoSize = true;
            this.flowLayoutPanel1.AutoSizeMode = System.Windows.Forms.AutoSizeMode.GrowAndShrink;
            this.flowLayoutPanel1.Controls.Add(this.LeftButton);
            this.flowLayoutPanel1.Controls.Add(this.DiffButton);
            this.flowLayoutPanel1.Controls.Add(this.RightButton);
            this.flowLayoutPanel1.Controls.Add(this.ApplyChangesButton);
            this.flowLayoutPanel1.Controls.Add(this.CopyContainerButton);
            this.flowLayoutPanel1.Controls.Add(this.btnSaveAll);
            this.flowLayoutPanel1.Controls.Add(this.btnViewCFGOnly);
            this.flowLayoutPanel1.Dock = System.Windows.Forms.DockStyle.Top;
            this.flowLayoutPanel1.Location = new System.Drawing.Point(0, 0);
            this.flowLayoutPanel1.Name = "flowLayoutPanel1";
            this.flowLayoutPanel1.Size = new System.Drawing.Size(995, 32);
            this.flowLayoutPanel1.TabIndex = 0;
            // 
            // LeftButton
            // 
            this.LeftButton.AutoSize = true;
            this.LeftButton.Location = new System.Drawing.Point(3, 3);
            this.LeftButton.Name = "LeftButton";
            this.LeftButton.Size = new System.Drawing.Size(56, 26);
            this.LeftButton.TabIndex = 0;
            this.LeftButton.Text = "Left";
            this.LeftButton.UseVisualStyleBackColor = true;
            this.LeftButton.CheckedChanged += new System.EventHandler(this.LeftButton_CheckedChanged);
            // 
            // DiffButton
            // 
            this.DiffButton.AutoSize = true;
            this.DiffButton.Checked = true;
            this.DiffButton.Location = new System.Drawing.Point(65, 3);
            this.DiffButton.Name = "DiffButton";
            this.DiffButton.Size = new System.Drawing.Size(54, 26);
            this.DiffButton.TabIndex = 1;
            this.DiffButton.TabStop = true;
            this.DiffButton.Text = "Diff";
            this.DiffButton.UseVisualStyleBackColor = true;
            this.DiffButton.CheckedChanged += new System.EventHandler(this.LeftButton_CheckedChanged);
            // 
            // RightButton
            // 
            this.RightButton.AutoSize = true;
            this.RightButton.Location = new System.Drawing.Point(125, 3);
            this.RightButton.Name = "RightButton";
            this.RightButton.Size = new System.Drawing.Size(63, 26);
            this.RightButton.TabIndex = 2;
            this.RightButton.Text = "Right";
            this.RightButton.UseVisualStyleBackColor = true;
            this.RightButton.CheckedChanged += new System.EventHandler(this.LeftButton_CheckedChanged);
            // 
            // ApplyChangesButton
            // 
            this.ApplyChangesButton.Enabled = false;
            this.ApplyChangesButton.Location = new System.Drawing.Point(194, 3);
            this.ApplyChangesButton.Name = "ApplyChangesButton";
            this.ApplyChangesButton.Size = new System.Drawing.Size(98, 23);
            this.ApplyChangesButton.TabIndex = 3;
            this.ApplyChangesButton.Text = "Apply Changes";
            this.ApplyChangesButton.UseVisualStyleBackColor = true;
            this.ApplyChangesButton.Click += new System.EventHandler(this.ApplyChangesButton_Click);
            // 
            // CopyContainerButton
            // 
            this.CopyContainerButton.Location = new System.Drawing.Point(298, 3);
            this.CopyContainerButton.Name = "CopyContainerButton";
            this.CopyContainerButton.Size = new System.Drawing.Size(98, 23);
            this.CopyContainerButton.TabIndex = 4;
            this.CopyContainerButton.Text = "Copy Container";
            this.CopyContainerButton.UseVisualStyleBackColor = true;
            this.CopyContainerButton.Click += new System.EventHandler(this.CopyContainerButton_Click);
            // 
            // btnSaveAll
            // 
            this.btnSaveAll.Location = new System.Drawing.Point(402, 3);
            this.btnSaveAll.Name = "btnSaveAll";
            this.btnSaveAll.Size = new System.Drawing.Size(107, 26);
            this.btnSaveAll.TabIndex = 5;
            this.btnSaveAll.Text = "SaveAllPasses";
            this.btnSaveAll.UseVisualStyleBackColor = true;
            this.btnSaveAll.Click += new System.EventHandler(this.btnSaveAll_Click);
            // 
            // btnViewCFGOnly
            // 
            this.btnViewCFGOnly.Location = new System.Drawing.Point(515, 3);
            this.btnViewCFGOnly.Name = "btnViewCFGOnly";
            this.btnViewCFGOnly.Size = new System.Drawing.Size(105, 26);
            this.btnViewCFGOnly.TabIndex = 6;
            this.btnViewCFGOnly.Text = "ViewCFGOnly";
            this.btnViewCFGOnly.UseVisualStyleBackColor = true;
            this.btnViewCFGOnly.Click += new System.EventHandler(this.btnViewCFGOnly_Click);
            // 
            // LogBox
            // 
            this.LogBox.Dock = System.Windows.Forms.DockStyle.Fill;
            this.LogBox.Location = new System.Drawing.Point(0, 0);
            this.LogBox.Name = "LogBox";
            this.LogBox.Size = new System.Drawing.Size(995, 73);
            this.LogBox.TabIndex = 0;
            this.LogBox.Text = "";
            // 
            // OptEditorForm
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(1497, 901);
            this.Controls.Add(this.TopContainer);
            this.Name = "OptEditorForm";
            this.Text = "Optimizer Editor";
            this.Load += new System.EventHandler(this.OptEditorForm_Load);
            this.TopContainer.Panel1.ResumeLayout(false);
            this.TopContainer.Panel2.ResumeLayout(false);
            ((System.ComponentModel.ISupportInitialize)(this.TopContainer)).EndInit();
            this.TopContainer.ResumeLayout(false);
            this.WorkContainer.Panel1.ResumeLayout(false);
            this.WorkContainer.Panel1.PerformLayout();
            this.WorkContainer.Panel2.ResumeLayout(false);
            ((System.ComponentModel.ISupportInitialize)(this.WorkContainer)).EndInit();
            this.WorkContainer.ResumeLayout(false);
            this.flowLayoutPanel1.ResumeLayout(false);
            this.flowLayoutPanel1.PerformLayout();
            this.ResumeLayout(false);

        }

        #endregion

        private System.Windows.Forms.SplitContainer TopContainer;
        private System.Windows.Forms.SplitContainer WorkContainer;
        private System.Windows.Forms.ListBox PassesListBox;
        private System.Windows.Forms.FlowLayoutPanel flowLayoutPanel1;
        private System.Windows.Forms.RadioButton LeftButton;
        private System.Windows.Forms.RadioButton DiffButton;
        private System.Windows.Forms.RadioButton RightButton;
        private System.Windows.Forms.Button ApplyChangesButton;
        private System.Windows.Forms.RichTextBox CodeBox;
        private System.Windows.Forms.RichTextBox LogBox;
        private System.Windows.Forms.Button CopyContainerButton;
        private System.Windows.Forms.Button btnSaveAll;
        private System.Windows.Forms.SaveFileDialog saveFileDialog1;
        private System.Windows.Forms.Button btnViewCFGOnly;
    }
}