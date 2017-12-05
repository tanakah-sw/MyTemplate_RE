using System;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Interop;
using System.Runtime.InteropServices;


namespace WPFPage
{

    public partial class TestPanel : UserControl
    {
        [DllImport("user32.dll")]
        public static extern int PostMessage(int hWnd, int msg, int wParam, IntPtr lParam);
        [DllImport("kernel32.dll")]
        public static extern int AddAtom(char[] lpstring);

        public static ViewModel lvm=null;

        public TestPanel(bool bcheck)
        {
            InitializeComponent();

            InteropTest.lbt = this;
            lvm = new ViewModel();
            DataContext = lvm;

            lvm.CheckTest = bcheck;
        }


        private void ClickButton(object sender, RoutedEventArgs e)
        {
            HwndSource source = (HwndSource)HwndSource.FromVisual(this);
            Button clicked = (Button)sender;
            lvm.Message = "ClickButton";
            PostMessage(source.Handle.ToInt32(), 0x0111, 0, (IntPtr)AddAtom(lvm.Message.ToCharArray()));
        }

        private void ClickCheck(object sender, RoutedEventArgs e)
        {
            HwndSource source = (HwndSource)HwndSource.FromVisual(this);
            if (lvm.CheckTest == false)
            {
                lvm.CheckTest = true;
                lvm.Message = "Checked";
            }
            else
            {
                lvm.CheckTest = false;
                lvm.Message = "Unchecked";
            }
            PostMessage(source.Handle.ToInt32(), 0x0111, 0, (IntPtr)AddAtom(lvm.Message.ToCharArray()));
        }
        
        public void UnCheck()
        {
            lvm.CheckTest = false;
        }
    }

    [ClassInterface(ClassInterfaceType.AutoDual)]
    public class InteropTest
    {
        public static TestPanel lbt;

        public void Uncheck()
        {
            lbt.UnCheck();
        }
    }
}