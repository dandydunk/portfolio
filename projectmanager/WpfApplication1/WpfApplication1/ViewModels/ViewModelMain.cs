using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.ComponentModel;
using System.Windows.Controls;

namespace WpfApplication1.ViewModels {
    public class ViewModelMain : INotifyPropertyChanged {
        UserControl _CurrentView = null;

        public UserControl CurrentView
        {
            get
            {
                if(null == _CurrentView) {
                    _CurrentView = new WpfApplication1.Views.UserControlManageProjects();
                    _CurrentView.DataContext = new ViewModelManageProjects(this);
                }

                return _CurrentView;
            }

            set
            {
                _CurrentView = value;
                OnPropertyChanged("CurrentView");
            }
        }

        public event PropertyChangedEventHandler PropertyChanged;
        public void OnPropertyChanged(string Property)
        {
            if (PropertyChanged != null)
            {
                PropertyChanged(this, new PropertyChangedEventArgs(Property));
            }
        }
    }
}
