from pythermalcomfort.models.pmv_ppd_iso import pmv_ppd_iso
from pythermalcomfort.utilities import v_relative, clo_dynamic_ashrae
from pythermalcomfort.utilities import clo_individual_garments
from pythermalcomfort.utilities import met_typical_tasks


class ThermalComfort:
    def __init__(self):
        self.tr = 26.6    # Mean radiant temperature (°C)
        v = 0.715  # Air velocity (m/s)

        garments = [
            'Long sleeve shirt (thin)', 'T-shirt', 'Thin trousers', 'Shoes or sandals'
        ]
        activity = 'Typing'

        icl = sum(clo_individual_garments[item] for item in garments)

        self.met = met_typical_tasks[activity]
        self.vr = v_relative(v, self.met)
        self.clo = clo_dynamic_ashrae(icl, self.met)

    def calculate_pmv(self, ta, rh):
        return pmv_ppd_iso(
            tdb=ta,
            tr=self.tr,
            vr=self.vr,
            rh=rh,
            met=self.met,
            clo=self.clo
        )['pmv']
