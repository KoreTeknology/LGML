import os
class Builder:
  rootPath = os.path.abspath(os.path.join(__file__,os.pardir,os.pardir))
  default_cfg_all = {
  "appName": "LGML",
  "njobs" : 1,
  "lgml_root_path" : rootPath,
  "build_cfg_name" : "Debug",

  }

  def __init__(self,cfg=None,actions = None):
    self.cfg = cfg or {};
    self.action = actions or ['build']
    self.verbose = "normal" # or quiet or verbose
    self.applyDefaultCfg(self.default_cfg_all)
    

  def applyDefaultCfg(self, default):
    for k in default:
      if not k in self.cfg:
        self.cfg[k] = default[k]

  def generateProductBaseName(self):
    import ProjucerUtils
    name =  self.cfg["appName"]+ "_v"+str(ProJucerUtils.getVersion())
    return name
  




