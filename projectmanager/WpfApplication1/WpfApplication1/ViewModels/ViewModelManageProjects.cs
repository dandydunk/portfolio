using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.ComponentModel;
using System.Windows.Input;


namespace WpfApplication1.ViewModels {
    public class ViewModelManageProjects {
        public ICommand NewProject {get; set; }

        public ViewModelManageProjects(ViewModelMain vmm) {
            NewProject = new CommandNewProject(vmm);
        }
    }

    public class CommandNewProject : ICommand {
        public event EventHandler CanExecuteChanged;
        ViewModelMain vmm = null;

        public CommandNewProject(ViewModelMain vmm) {
            this.vmm = vmm;
        }

        public void Execute(object obj) {
            System.Windows.Controls.UserControl uc = new WpfApplication1.Views.UserControlLoading();
            vmm.CurrentView = uc;
        }

        public bool CanExecute(object obj) {
            return true;
        }
    }
}
