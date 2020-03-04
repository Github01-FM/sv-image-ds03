def get_speex_fpu_setting(bb, d):
     if bb.data.getVar('TARGET_FPU', d, 1) in [ 'vfp' ]:
             return "--enable-float-api --disable-vbr"
     return "--enable-fixed-point --disable-float-api --disable-vbr"
