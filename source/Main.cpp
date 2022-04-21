#include "plugin.h"
#include "CMenuManager.h"
#include "CDamageManager.h"
#include "CVehicleModelInfo.h"
#include "Utility.h"
#include "CFont.h"

using namespace plugin;

class HoodDetach {
public:
	HoodDetach() {
		Events::vehicleRenderEvent += [](CVehicle* veh) {
			CAutomobile* autoMobile = static_cast<CAutomobile*>(veh);
			CPlayerPed* playa = FindPlayerPed();
			int node = CAR_BONNET;
			RwFrame* bonnetFrame = autoMobile->m_aCarNodes[node];
			RwMatrix* bonnetMatrix = RwFrameGetMatrix(bonnetFrame);
			CDamageManager damage = autoMobile->m_carDamage;
			CDoor& bonnet = autoMobile->m_aDoors[DOOR_BONNET];
			const float maxAngle = 0.8f;

			if (!playa || !autoMobile)
				return;

			if (bonnetFrame && damage.GetDoorStatus(DOOR_BONNET) == DAMSTATE_DAMAGED) {
				CVector speed = autoMobile->m_vecMoveSpeed + autoMobile->m_vecFrictionMoveForce;
				speed += CrossProduct(autoMobile->m_vecTurnSpeed + autoMobile->m_vecFrictionTurnForce, CVector(1.0f, 0.0f, 0.0f));

				CVector vecSpeedDiff = bonnet.field_18 - speed;
				vecSpeedDiff = Multiply3x3(vecSpeedDiff, autoMobile->m_matrix);

				float speedDiff = -(vecSpeedDiff.y + vecSpeedDiff.z);
				if (abs(speedDiff) > 0.002f) {
					bonnet.m_fAngVel += speedDiff;
					bonnet.m_fAngVel = clamp(bonnet.m_fAngVel, -0.3f, 0.3f);
				}

				bonnet.m_fAngle += bonnet.m_fAngVel;
				bonnet.m_fAngle = clamp(bonnet.m_fAngle, 0.0f, maxAngle);
				bonnet.m_bState = 0;

				bonnet.field_18 = speed;

				CMatrix mat;
				CVector pos = mat.pos;

				mat.m_pAttachMatrix = NULL;
				mat.Attach(bonnetMatrix, false);

				mat.SetRotateXOnly(autoMobile->m_aDoors[DOOR_BONNET].m_fAngle);
				mat.UpdateRW();

				if (autoMobile->m_vecMoveSpeed.Magnitude() > 0.5f &&
					autoMobile->m_aDoors[DOOR_BONNET].m_fAngle * 0.5f <= maxAngle) {
					autoMobile->SpawnFlyingComponent(node, COMPGROUP_BONNET);
					autoMobile->SetComponentVisibility(bonnetFrame, 0);
					autoMobile->m_carDamage.SetDoorStatus(DOOR_BONNET, DAMSTATE_NOTPRESENT);
				}
			}
		};
	}
} hoodDetach;
