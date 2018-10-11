
from building import *
import rtconfig

cwd = GetCurrentDir()

src = []

src += Glob('*.c')
CPPPATH = [cwd]

group = DefineGroup('flex_button', src, depend = [], CPPPATH = CPPPATH)

Return('group')
