import numpy as np
from scipy import stats

# https://stackoverflow.com/a/34278277/17332200
# http://stats.org.uk/statistical-inference/Newcombe1998.pdf
def calc_propci_wilson_cc(count: int, nobs: int, alpha: float=0.05) -> float:
    # get confidence limits for proportion
    # using wilson score method w/ cont correction
    # i.e. Method 4 in Newcombe [1];
    # verified via Table 1
    n = nobs
    p = count/n
    q = 1.-p
    z = stats.norm.isf(alpha / 2.)
    z2 = z**2
    denom = 2*(n+z2)
    num = 2.*n*p+z2-1.-z*np.sqrt(z2-2-1./n+4*p*(n*q+1))
    ci_l = num/denom
    num = 2.*n*p+z2+1.+z*np.sqrt(z2+2-1./n+4*p*(n*q-1))
    ci_u = num/denom
    if p == 0:
        ci_l = 0.
    elif p == 1:
        ci_u = 1.
    return ci_l, ci_u
