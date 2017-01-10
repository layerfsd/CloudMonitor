
file_list = {
    # add all the files under "default_root"
    "default_root": "Release",

    # these files locate in Release
    # But when don't need them when distribute a new version
    "exclude_files": [
        'DATA\\keywords.txt',
        'DATA\\auth.dat',
        'DATA\\hashlist.txt',
        'DATA\\VERSION',
        'UPDATE_CHECKED',
        'RUNNING_FLAG',
        'clean.sh',
        'Service.txt',
    ],

    "exclude_dirs": [
        'log',     # runtime generated log
        'TMP',     # runtime generated files
    ],

    "skip_suffix": [
        '.iobj',
        '.ipdb',
        '.exp',
        '.bat',
    ],

    # parallel to "default_root"
    "appendix_files": [
        'x64\\Release\\HooKeyboard-64.dll',
        'x64\\Release\\MonitorService-64.exe',
    ]
}
