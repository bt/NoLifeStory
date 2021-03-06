////////////////////////////////////////////////////
// This file is part of NoLifeStory.              //
// Please see SuperGlobal.h for more information. //
////////////////////////////////////////////////////
#include "Global.h"

NLS::LoginPlayer::LoginPlayer() {
	state = "stand1";
	frame = 0;
	delay = 0;
	emote = "default";
	emoted = 0;
	emotef = 0;
	skin = 1;
	face = 20000;
	hair = 30000;
	level = 8;
	name = "[EPIC]NLSER";
	nametag.Set(name, NameTag::Normal);
	for (int8_t i = 0; i < 20; i++) {
		SetItemBySlot(i, 0);
	}
	clothes = 1040036;
	x = y = 0;
	f = true;
}

void NLS::LoginPlayer::SetItemBySlot(int8_t slotid, int32_t itemid) {
	switch (slotid) {
	case 1: cap = itemid; break;
	case 2: forehead = itemid; break;
	case 3: eyeacc = itemid; break;
	case 4: earacc = itemid; break;
	case 5: clothes = itemid; break;
	case 6: pants = itemid; break;
	case 7: shoes = itemid; break;
	case 8: gloves = itemid; break;
	case 9: mantle = itemid; break;
	case 10: shield = itemid; break;
	case 11: weapon = itemid; break;
	}
	if (itemid != 0) {
		cout << "Item " << itemid << " (" << NLS::stringItems[itemid] << ") on slot " << (int)slotid << endl;
	}
}

void NLS::LoginPlayer::Draw() {
	if (emote != "default") {
		emoted += Time::delta*1000;
		emotee += Time::delta*1000;
		int d = WZ["Character"]["Face"][tostring(face, 8)][emote][emotef]["delay"];
		if (emoted > d) {
			emotef++;
			emoted = 0;
			if (!WZ["Character"]["Face"][tostring(face, 8)][emote][emotef]) {
				emotef = 0;
				if (emote == "blink") {
					emote = "default";
				}
			}
		}
	} else {
		if (rand()%5000 < Time::delta*1000) {
			emote = "blink";
			emotee = 0;
			emoted = 0;
			emotef = 0;
		}
	}
	delay += Time::delta*1000;

	auto skinData = WZ["Character"]["00002"+tostring(skin, 3)];
	auto headData = WZ["Character"]["00012"+tostring(skin, 3)];

	int d = skinData[state][frame]["delay"];
	static bool weird = false;
	if (delay > d) {
		delay = 0;
		if (weird) {
			frame--;
			if (!frame)	weird = false;
		} else {
			frame++;
			weird = false;
		}
	}
	if (!skinData[state][frame]) {
		frame = 0;
	}

	Node zmap = WZ["zmap"];
	vector<Node> parts;
	parts.push_back(skinData[state][frame]);
	parts.push_back(headData[state][frame]);
	parts.push_back(WZ["Character"]["Hair"][tostring(hair, 8)][state][frame]);
	//You jelly of my braceless ifs?
	if ((int)skinData[state][frame]["face"])
		if (emote == "default") parts.push_back(WZ["Character"]["Face"][tostring(face, 8)][emote]);
		else parts.push_back(WZ["Character"]["Face"][tostring(face, 8)][emote][emotef]);
	if (clothes)
		if (clothes < 1050000) parts.push_back(WZ["Character"]["Coat"][tostring(clothes, 8)][state][tostring(frame)]);
		else parts.push_back(WZ["Character"]["Longcoat"][tostring(clothes, 8)][state][tostring(frame)]);
	if (pants) parts.push_back(WZ["Character"]["Pants"][tostring(pants, 8)][state][tostring(frame)]);
	if (cap) parts.push_back(WZ["Character"]["Cap"][tostring(cap, 8)][state][tostring(frame)]);
	if (mantle) parts.push_back(WZ["Character"]["Cape"][tostring(mantle, 8)][state][tostring(frame)]);
	if (shoes) parts.push_back(WZ["Character"]["Shoes"][tostring(shoes, 8)][state][tostring(frame)]);
	if (gloves) parts.push_back(WZ["Character"]["Gloves"][tostring(gloves, 8)][state][tostring(frame)]);
	if (weapon) parts.push_back(WZ["Character"]["Weapon"][tostring(weapon, 8)][state][tostring(frame)]);
	if (shield) parts.push_back(WZ["Character"]["Shield"][tostring(shield, 8)][state][tostring(frame)]);

	struct part {
		Sprite spr;
		int z;
		double x;
		double y;
	};
	vector<part> sparts;
	map<string, sf::Vector2<double>> maps;
	Node base = skinData[state][frame];
	if (f) {
		maps["navel"].x = x-(double)base["body"]["map"]["navel"]["x"];
	} else {
		maps["navel"].x = x+(double)base["body"]["map"]["navel"]["x"];
	}
	maps["navel"].y = y+(double)base["body"]["map"]["navel"]["y"];
	for (auto it = parts.begin(); it != parts.end(); ++it) {
		Node n = *it;
		if (!n) continue;
		if (n["action"]) n = n[".."][".."][n["action"]][n["frame"]];
		for (auto it = n.begin(); it != n.end(); ++it) {
			string name = it->first;
			Node nn = it->second;
			if (!nn) continue;
			Sprite s = nn;
			if (!s.data) continue;
			if (cap and (name == "hairOverHead" or name == "backHair")) continue;
			part np;
			np.spr = s;
			np.z = zmap[nn["z"]];
			np.x = x;
			np.y = y;
			nn = nn["map"];
			if (!nn) continue;
			for (auto it = nn.rbegin(); it != nn.rend(); ++it) {
				string mname = it->first;
				Node mn = it->second;
				if (maps.find(mname) == maps.end()) {
					if (f) {
						maps[mname].x = np.x-(double)mn["x"];
					} else {
						maps[mname].x = np.x+(double)mn["x"];
					}
					maps[mname].y = np.y+(double)mn["y"];
				} else {
					if (f) {
						np.x = maps[mname].x+(double)mn["x"];
					} else {
						np.x = maps[mname].x-(double)mn["x"];
					}
					np.y = maps[mname].y-(double)mn["y"];
				}
			}
			sparts.push_back(np);
		}
	}
	sort(sparts.begin(), sparts.end(), [&](const part& a, const part& b){return a.z>b.z;});
	for (auto it = sparts.begin(); it != sparts.end(); it++) {
		it->spr.Draw(it->x, it->y, f);
	}

	nametag.Draw(x, y);
}