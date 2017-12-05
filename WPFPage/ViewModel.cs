using System;
using System.ComponentModel;
using System.Runtime.CompilerServices;

namespace WPFPage
{
    public class ViewModel : INotifyPropertyChanged
    {
        public event PropertyChangedEventHandler PropertyChanged;
        private void NotifyPropertyChanged([CallerMemberName] String propertyName = "")
        {
            if (PropertyChanged != null)
            {
                PropertyChanged(this, new PropertyChangedEventArgs(propertyName));
            }
        }

        private string message = "";
        public string Message
        {
            get
            {
                return this.message;
            }
            set
            {
                if (value != this.message)
                {
                    this.message = value;
                    NotifyPropertyChanged();
                }
            }
        }

        private bool checkTest = false;
        public bool CheckTest
        {
            get
            {
                return this.checkTest;
            }
            set
            {
                if (value != this.checkTest)
                {
                    this.checkTest = value;
                    NotifyPropertyChanged();
                }
            }
        }
    }
}