///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// GoToDialog.Designer..cs                                                   //
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
// This file is distributed under the University of Illinois Open Source     //
// License. See LICENSE.TXT for details.                                     //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

namespace MainNs
{
    partial class GoToDialog
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
            this.GoToLabel = new System.Windows.Forms.Label();
            this.GoToBox = new System.Windows.Forms.TextBox();
            this.OKButton = new System.Windows.Forms.Button();
            this.TheCancelButton = new System.Windows.Forms.Button();
            this.SuspendLayout();
            // 
            // GoToLabel
            // 
            this.GoToLabel.AutoSize = true;
            this.GoToLabel.Location = new System.Drawing.Point(13, 13);
            this.GoToLabel.Name = "GoToLabel";
            this.GoToLabel.Size = new System.Drawing.Size(79, 20);
            this.GoToLabel.TabIndex = 0;
            this.GoToLabel.Text = "GoToLine";
            // 
            // GoToBox
            // 
            this.GoToBox.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.GoToBox.Location = new System.Drawing.Point(12, 36);
            this.GoToBox.Name = "GoToBox";
            this.GoToBox.Size = new System.Drawing.Size(290, 26);
            this.GoToBox.TabIndex = 1;
            this.GoToBox.TextChanged += new System.EventHandler(this.textBox1_TextChanged);
            // 
            // OKButton
            // 
            this.OKButton.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right)));
            this.OKButton.DialogResult = System.Windows.Forms.DialogResult.OK;
            this.OKButton.Location = new System.Drawing.Point(100, 68);
            this.OKButton.Name = "OKButton";
            this.OKButton.Size = new System.Drawing.Size(99, 28);
            this.OKButton.TabIndex = 2;
            this.OKButton.Text = "OK";
            this.OKButton.UseVisualStyleBackColor = true;
            // 
            // TheCancelButton
            // 
            this.TheCancelButton.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right)));
            this.TheCancelButton.DialogResult = System.Windows.Forms.DialogResult.Cancel;
            this.TheCancelButton.Location = new System.Drawing.Point(205, 68);
            this.TheCancelButton.Name = "TheCancelButton";
            this.TheCancelButton.Size = new System.Drawing.Size(97, 28);
            this.TheCancelButton.TabIndex = 3;
            this.TheCancelButton.Text = "Cancel";
            this.TheCancelButton.UseVisualStyleBackColor = true;
            // 
            // GoToDialog
            // 
            this.AcceptButton = this.OKButton;
            this.AutoScaleDimensions = new System.Drawing.SizeF(9F, 20F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.CancelButton = this.TheCancelButton;
            this.ClientSize = new System.Drawing.Size(314, 106);
            this.Controls.Add(this.TheCancelButton);
            this.Controls.Add(this.OKButton);
            this.Controls.Add(this.GoToBox);
            this.Controls.Add(this.GoToLabel);
            this.Name = "GoToDialog";
            this.StartPosition = System.Windows.Forms.FormStartPosition.CenterParent;
            this.Text = "Go To Line";
            this.Load += new System.EventHandler(this.GoToDialog_Load);
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.Label GoToLabel;
        private System.Windows.Forms.TextBox GoToBox;
        private System.Windows.Forms.Button OKButton;
        private System.Windows.Forms.Button TheCancelButton;
    }
}