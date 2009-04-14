

using System;
using System.Collections;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;
using System.Threading;


namespace ConsoleSeparate
{
    public class frmConsole : Form
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
            this.components = new System.ComponentModel.Container();
            this.txtOut = new System.Windows.Forms.TextBox();
            this.txtErr = new System.Windows.Forms.TextBox();
            this.txtIn = new System.Windows.Forms.TextBox();
            this.tmrUpdate = new System.Windows.Forms.Timer(this.components);
            this.txtOutHistory = new System.Windows.Forms.TextBox();
            this.txtErrHistory = new System.Windows.Forms.TextBox();
            this.SuspendLayout();
            // 
            // txtOut
            // 
            this.txtOut.Font = new System.Drawing.Font("Courier New", 8.25F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.txtOut.ForeColor = System.Drawing.SystemColors.ActiveCaption;
            this.txtOut.Location = new System.Drawing.Point(7, 153);
            this.txtOut.Multiline = true;
            this.txtOut.Name = "txtOut";
            this.txtOut.ReadOnly = true;
            this.txtOut.ScrollBars = System.Windows.Forms.ScrollBars.Both;
            this.txtOut.Size = new System.Drawing.Size(309, 121);
            this.txtOut.TabIndex = 0;
            this.txtOut.WordWrap = false;
            // 
            // txtErr
            // 
            this.txtErr.Font = new System.Drawing.Font("Courier New", 8.25F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.txtErr.ForeColor = System.Drawing.SystemColors.ActiveCaption;
            this.txtErr.Location = new System.Drawing.Point(328, 135);
            this.txtErr.Multiline = true;
            this.txtErr.Name = "txtErr";
            this.txtErr.ReadOnly = true;
            this.txtErr.ScrollBars = System.Windows.Forms.ScrollBars.Both;
            this.txtErr.Size = new System.Drawing.Size(215, 112);
            this.txtErr.TabIndex = 1;
            this.txtErr.WordWrap = false;
            //this.txtErr.PreviewKeyDown += new System.Windows.Forms.PreviewKeyDownEventHandler(this.txtErr_PreviewKeyDown);
            this.txtErr.KeyDown += new System.Windows.Forms.KeyEventHandler(this.txtErr_KeyDown);

            // 
            // txtIn
            // 
            this.txtIn.AcceptsReturn = true;
            this.txtIn.Font = new System.Drawing.Font("Microsoft Sans Serif", 12F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.txtIn.Location = new System.Drawing.Point(328, 253);
            this.txtIn.Name = "txtIn";
            this.txtIn.Size = new System.Drawing.Size(214, 26);
            this.txtIn.TabIndex = 2;
            this.txtIn.KeyPress += new System.Windows.Forms.KeyPressEventHandler(this.txtIn_KeyPress);
            // 
            // tmrUpdate
            // 
            this.tmrUpdate.Tick += new System.EventHandler(this.tmrUpdate_Tick);
            // 
            // txtOutHistory
            // 
            this.txtOutHistory.Font = new System.Drawing.Font("Courier New", 8.25F, System.Drawing.FontStyle.Italic, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.txtOutHistory.Location = new System.Drawing.Point(7, 8);
            this.txtOutHistory.Multiline = true;
            this.txtOutHistory.Name = "txtOutHistory";
            this.txtOutHistory.ReadOnly = true;
            this.txtOutHistory.ScrollBars = System.Windows.Forms.ScrollBars.Both;
            this.txtOutHistory.Size = new System.Drawing.Size(309, 139);
            this.txtOutHistory.TabIndex = 3;
            this.txtOutHistory.WordWrap = false;
            // 
            // txtErrHistory
            // 
            this.txtErrHistory.Font = new System.Drawing.Font("Courier New", 8.25F, System.Drawing.FontStyle.Italic, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.txtErrHistory.Location = new System.Drawing.Point(328, 8);
            this.txtErrHistory.Multiline = true;
            this.txtErrHistory.Name = "txtErrHistory";
            this.txtErrHistory.ReadOnly = true;
            this.txtErrHistory.ScrollBars = System.Windows.Forms.ScrollBars.Both;
            this.txtErrHistory.Size = new System.Drawing.Size(215, 112);
            this.txtErrHistory.TabIndex = 4;
            this.txtErrHistory.WordWrap = false;
            // 
            // frmConsole
            // 
            //this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            //this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(551, 286);
            this.Controls.Add(this.txtErrHistory);
            this.Controls.Add(this.txtOutHistory);
            this.Controls.Add(this.txtIn);
            this.Controls.Add(this.txtErr);
            this.Controls.Add(this.txtOut);
            this.Name = "frmConsole";
            this.WindowState = System.Windows.Forms.FormWindowState.Maximized;
			this.Closed += new System.EventHandler(this.frmConsole_FormClosed);
			this.SizeChanged += new System.EventHandler(this.frmConsole_SizeChanged);
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.TextBox txtOut;
        private System.Windows.Forms.TextBox txtErr;
        private System.Windows.Forms.TextBox txtIn;
        private System.Windows.Forms.Timer tmrUpdate;
        private System.Windows.Forms.TextBox txtOutHistory;
        private System.Windows.Forms.TextBox txtErrHistory;
	
	
	
        public CmdController myProcessInterface;
        private Thread launchIndependant;

        const int ROTATIONLINES = 50;

        public frmConsole(string[] args)
        {
            InitializeComponent();
            CleanUpResize();        
            
            string strArgs = string.Empty;
            //PREPROCESS ARGUMENTS
            if (args.Length >= 2)
            {
                for (int i=1;i<args.Length;++i)
                {
                    strArgs += args[i];
                }
            }

            if (args.Length > 0)
            {

                this.Text = args[0];

                myProcessInterface = new CmdController(args[0], strArgs);

                launchIndependant = new Thread(new ThreadStart(myProcessInterface.Begin));
                launchIndependant.Start();
                


                new MethodInvoker(myProcessInterface.AutoPollStdOut).BeginInvoke(null, null);
                new MethodInvoker(myProcessInterface.AutoPollStdErr).BeginInvoke(null, null);


                tmrUpdate.Enabled = true;
            }
        }

        private void tmrUpdate_Tick(object sender, EventArgs e)
        {
            string newString = myProcessInterface.ReadMoreStdOutput();
            txtOut.Text += newString;
            if (newString.Length > 0)
            {
                txtOut.SelectionStart = txtOut.Text.Length;
                txtOut.ScrollToCaret();
            }
            

            
            newString = myProcessInterface.ReadMoreStdError();
            txtErr.Text += newString;
            if (newString.Length > 0)
            {
                txtErr.SelectionStart = txtErr.Text.Length;
                txtErr.ScrollToCaret();
            }
        }

        protected void SendCommand(string commandText)
        {
            RotateTextBoxes();

            myProcessInterface.SendLine(commandText);
            txtErr.Text += commandText + '\r' + '\n';
            txtIn.Clear();
        }

        private void txtIn_KeyPress(object sender, KeyPressEventArgs e)
        {
            tmrUpdate.Stop();
            tmrUpdate.Enabled = false;

            if ( (e.KeyChar == '\r') || (e.KeyChar == '\n') )
            {
                SendCommand(txtIn.Text);

                e.Handled = true;
            }

            tmrUpdate.Enabled = true;
            tmrUpdate.Start();
        }

        ~frmConsole()
        {
            myProcessInterface.EndProgram();
            launchIndependant.Join();
        }

        protected void RotateTextBoxes()
        {
		/*
            if (txtOutHistory.Lines.LongLength > ROTATIONLINES)
            {
                txtOutHistory.Text = txtOut.Text.Remove(0, ROTATIONLINES);   
            }
            if (txtErrHistory.Lines.LongLength > ROTATIONLINES)
            {
                txtErrHistory.Text = txtErr.Text.Remove(0, ROTATIONLINES);
            }
*/
            txtOutHistory.Text += txtOut.Text;
            txtErrHistory.Text += txtErr.Text;

            txtOut.Text = string.Empty;
            txtErr.Text = string.Empty;

            txtOutHistory.SelectionStart = txtOutHistory.Text.Length;
            txtOutHistory.ScrollToCaret();

            txtErrHistory.SelectionStart = txtErrHistory.Text.Length;
            txtErrHistory.ScrollToCaret();
        }

        protected void CleanUpResize()
        {

            int borderWidth = (this.Width - this.ClientSize.Width) / 2;
            int titlebarHeight = this.Height - this.ClientSize.Height - 2 * borderWidth;

            int gapSize = txtOut.Left;
            const int numerator = 1;
            const int denominator = 2;

            int newWidth = this.Width - 3 * gapSize - borderWidth * 2;
            int newHeight = this.Height - 3 * gapSize - borderWidth * 2 - titlebarHeight;
            int newHeightErr = newHeight - gapSize - txtIn.Height;


            txtErr.Height = newHeightErr * numerator / denominator;
            txtErrHistory.Height = newHeightErr - txtErr.Height;
            
            txtOutHistory.Height = txtErrHistory.Height;
            txtOut.Height = newHeight - txtOutHistory.Height;

            txtOutHistory.Width = newWidth * numerator / denominator;
            txtOut.Width = txtOutHistory.Width;
            txtErrHistory.Left = txtOut.Width + txtOut.Left + gapSize;
            txtErrHistory.Width = newWidth - txtOut.Width;
            txtErr.Left = txtErrHistory.Left;
            txtErr.Width = txtErrHistory.Width;

            txtErr.Top = txtErrHistory.Top + txtErrHistory.Height + gapSize;
            txtOut.Top = txtErr.Top;


            txtIn.Left = txtErr.Left;
            txtIn.Width = txtErr.Width;
            txtIn.Top = txtErr.Top + txtErr.Height + gapSize;
        }

        private void frmConsole_SizeChanged(object sender, EventArgs e)
        {
            CleanUpResize();
        }

        private void frmConsole_DoubleClick(object sender, EventArgs e)
        {
            myProcessInterface.SendLine("fold");
        }

		private void txtErr_KeyDown(object sender, KeyEventArgs e)
        {
		txtIn.Text = ((char)(e.KeyValue)).ToString().ToLower();
            txtIn.SelectionStart = 1;
            txtIn.Focus();
        }

        private void frmConsole_FormClosed(object sender, System.EventArgs e)
        {
            myProcessInterface.EndProgram();
        }



    }
}