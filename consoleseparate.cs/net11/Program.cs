using System;
using System.Collections;
using System.Windows.Forms;

namespace ConsoleSeparate
{
    class Program
    {
        /// <summary>
        /// The main entry point for the application.
        /// </summary>
        [STAThread]
        static void Main(string[] args)
        {
            Application.EnableVisualStyles();
            //Application.SetCompatibleTextRenderingDefault(false);
            Application.Run(new frmConsole(args));
        }
    }
}