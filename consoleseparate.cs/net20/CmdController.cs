using System;
//using System.Collections.Generic;
using System.Text.RegularExpressions;
using System.Text;
using System.IO;
using System.ComponentModel;
using System.Diagnostics;
using System.Threading;





namespace ConsoleSeparate
{
    public class CmdController
    {
        protected Process targetExe;
        protected StreamWriter cmdStdIn;
        protected StreamReader cmdStdOut;
        protected StreamReader cmdStdErr;

        bool bThreadPolling;

        private StringBuilder cmdStdOutBuffer;
        private StringBuilder cmdStdErrBuffer;

        public void AutoPollStdOut()
        {
            PollMonitorStream( ref cmdStdOut , ref cmdStdOutBuffer );
        }

        public void AutoPollStdErr()
        {
            PollMonitorStream( ref cmdStdErr , ref cmdStdErrBuffer );
        }


        protected virtual void PollMonitorStream(ref StreamReader x, ref StringBuilder destString)
        {
            while (x == null) ;

            int readChar;
            //string str;
            //while ((str = x.ReadLine()) != null)
            while ((readChar = x.Read()) != -1)
            {
                destString.Append((char)readChar);
                //destString.Append(str);
                //destString.Append('\r');
                //destString.Append('\n');
                //FireAsync(StdOutReceived, this, new DataReceivedEventArgs(str));
                Thread.Sleep(1);
            }
        }


        private static void ExtractCmdArgs( string sysCmdLine, out string sysCmd, out string sysArg )
        {
            Regex extractor = new Regex(@"^\s*((""[^""]+"")|('[^']+')|([^\s]+))",RegexOptions.Compiled);
            Match results = extractor.Match(sysCmdLine);
            if (results.Success)
            {
                sysCmd = results.ToString();
                sysCmdLine = sysCmdLine.Remove(0, sysCmd.Length);
                sysCmdLine.TrimStart();
                sysArg = sysCmdLine;
            }
            else
            {
                sysCmd = sysCmdLine;
                sysArg = string.Empty;
            }
        }
           
        public CmdController( string sysCmd, string sysArg )
        {
            if (sysArg.Length == 0)
            {
                string trialCmd;
                string trialArg;

                ExtractCmdArgs(sysCmd, out trialCmd, out trialArg);
                sysCmd = trialCmd;
                sysArg = trialArg;
            }

            // http://msdn.microsoft.com/library/default.asp?url=/library/en-us/cpref/html/frlrfsystemdiagnosticsprocessclassstandardinputtopic.asp
            targetExe = new Process();
            //targetExe.EnableRaisingEvents = true;
            if (sysArg.Length > 0)
            {
                targetExe.StartInfo.Arguments = sysArg; // http://msdn.microsoft.com/library/default.asp?url=/library/en-us/cpref/html/frlrfSystemDiagnosticsProcessStartInfoPropertiesTopic.asp
            }
            targetExe.StartInfo.FileName = sysCmd;

            targetExe.StartInfo.UseShellExecute = false; //So that we can redirect input?
            //targetExe.StartInfo.CreateNoWindow = true;
            
            targetExe.StartInfo.WorkingDirectory = Environment.CurrentDirectory;

        }

        public void Begin()
        {
            targetExe.StartInfo.RedirectStandardOutput = true;
            targetExe.StartInfo.RedirectStandardError = true;
            targetExe.StartInfo.RedirectStandardInput = true;
            

            targetExe.Start();

            cmdStdIn = targetExe.StandardInput;
            cmdStdOut = targetExe.StandardOutput;
            cmdStdErr = targetExe.StandardError;

            cmdStdErrBuffer = new StringBuilder();
            cmdStdOutBuffer = new StringBuilder();
            
            bThreadPolling = true;
            

        }

        public string ReadMoreStdError()
        {
            if (cmdStdErr == null) return string.Empty;
            string strData = cmdStdErrBuffer.ToString();
            cmdStdErrBuffer.Remove(0, strData.Length);
            return strData;
        }

        public string ReadMoreStdOutput()
        {
            if (cmdStdOut == null) return string.Empty;

            string strTee = cmdStdOutBuffer.ToString();
            cmdStdOutBuffer.Remove(0, strTee.Length);
            System.Console.Out.Write(strTee);
            return strTee;
        }

        public void SendLine(string sendout)
        {
            cmdStdIn.WriteLine(sendout);
            cmdStdIn.Flush();
        }

        public void EndProgram()
        {
            bThreadPolling = false;

            if( !(targetExe.HasExited) ) targetExe.Kill();
        }
        
        ~CmdController()
        {
            EndProgram();
        }
        

    }

}
