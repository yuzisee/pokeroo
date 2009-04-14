

using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;
using System.Threading;


namespace ConsoleSeparate
{
    public partial class frmConsole : Form
    {
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
            if (txtOutHistory.Lines.LongLength > ROTATIONLINES)
            {
                txtOutHistory.Text = txtOut.Text.Remove(0, ROTATIONLINES);   
            }
            if (txtErrHistory.Lines.LongLength > ROTATIONLINES)
            {
                txtErrHistory.Text = txtErr.Text.Remove(0, ROTATIONLINES);
            }

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

        private void txtErr_PreviewKeyDown(object sender, PreviewKeyDownEventArgs e)
        {
            txtIn.Text = ((char)(e.KeyValue)).ToString().ToLowerInvariant();
            txtIn.SelectionStart = 1;
            txtIn.Focus();
        }

        private void frmConsole_FormClosed(object sender, FormClosedEventArgs e)
        {
            myProcessInterface.EndProgram();
        }


    }
}