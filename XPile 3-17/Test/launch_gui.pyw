# launch_gui.pyw — Launch UniLogic Project Manager (no console window)
import sys
import os
import traceback

_dir = os.path.dirname(os.path.abspath(__file__))
_log = os.path.join(_dir, "GUI", "gui_error.log")
os.chdir(_dir)
sys.path.insert(0, os.path.join(_dir, "GUI"))

try:
    from project_manager import main
    main()
except SystemExit:
    pass
except BaseException:
    with open(_log, "w") as f:
        traceback.print_exc(file=f)
