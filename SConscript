
from building import *
import rtconfig

cwd = GetCurrentDir()

src = []

src += Split('''
flexible_button.c
''')

if GetDepend(['PKG_USING_FLEXIBLE_BUTTON_DEMO']):
    src += Glob("examples/demo_rtt_iotboard.c")

CPPPATH = [cwd]

group = DefineGroup('flex_button', src, depend = ['PKG_USING_FLEXIBLE_BUTTON'], CPPPATH = CPPPATH)

Return('group')
