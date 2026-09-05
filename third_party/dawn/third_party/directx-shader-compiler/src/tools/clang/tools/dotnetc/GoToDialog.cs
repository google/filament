///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// GoToDialog.cs                                                             //
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
// This file is distributed under the University of Illinois Open Source     //
// License. See LICENSE.TXT for details.                                     //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace MainNs
{
    public partial class GoToDialog : Form
    {
        public GoToDialog()
        {
            InitializeComponent();
        }

        private void textBox1_TextChanged(object sender, EventArgs e)
        {

        }

        private void GoToDialog_Load(object sender, EventArgs e)
        {

        }

        public int LineNumber
        {
            get
            {
                int result;
                if (!int.TryParse(this.GoToBox.Text, out result))
                {
                    result = 0;
                }
                return result;
            }
            set
            {
                this.GoToBox.Text = value.ToString();
            }
        }

        private int maxLineNumber;
        public int MaxLineNumber
        {
            get
            {
                return this.maxLineNumber;
            }
            set
            {
                this.maxLineNumber = value;
                this.GoToLabel.Text = "&Line number (1 - " + value.ToString() + "):";
            }
        }
    }
}
