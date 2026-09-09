///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// FindDialog.cs                                                             //
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
// This file is distributed under the University of Illinois Open Source     //
// License. See LICENSE.TXT for details.                                     //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

using System;
using System.Windows.Forms;

namespace MainNs
{
    public partial class FindDialog : Form
    {

        public FindDialog()
        {
            this.InitializeComponent();
        }

        public string FindText
        {
            get { return this.findControl.Text; }
            set { this.findControl.Text = value; }
        }

        public TextBoxBase Target { get; set; }

        private void CancelButton_Click(object sender, EventArgs e)
        {
            this.Close();
        }

        private void FindButton_Click(object sender, EventArgs e)
        {
            string findText = this.FindText;
            if (String.IsNullOrEmpty(findText)) return;

            TextBoxBase target = this.Target;
            if (target == null) return;

            target.HideSelection = false;

            int start = target.SelectionStart;
            int end = start + target.SelectionLength;
            int nextIndex = target.Text.IndexOf(findText, end);
            if (nextIndex >= 0)
            {
                target.Select(nextIndex, findText.Length);
            }
        }

        private void FindDialog_Load(object sender, EventArgs e)
        {
            this.ActiveControl = this.findControl;
        }
    }
}
