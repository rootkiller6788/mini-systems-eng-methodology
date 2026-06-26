import os
BASE = os.getcwd()
def w(p,c):
  with open(p,"w",encoding="utf-8") as f: f.write(c)
def a(p,c):
  with open(p,"a",encoding="utf-8") as f: f.write(c)
