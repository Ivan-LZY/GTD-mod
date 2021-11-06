import csv
import cv2
import shutil

def plot_veh(img, csv_file):
    vehid = 0 #skip header
    bb_x = []
    bb_y=[]
    occlu_v2m = []
    occlu_t = []
    occlu_v2obj = []
    veh_dist = []
    bbwin_x=[]
    bbwin_y=[]
    bbwin_x2=[]
    bbwin_y2=[]
    count = 0
    for row in csv_file:
        if count == 0:
            count+=1
            continue
        vehid_new = row[2]
        if vehid_new!=vehid:
            bb_x.append(int(float(row[3])))
            bb_y.append(int(float(row[4])))
            occlu_t.append(int(float(row[5])))
            occlu_v2obj.append(int(float(row[6])))
            occlu_v2m.append(int(float(row[7])))
            bbwin_x.append(int(float(row[8])))
            bbwin_y.append(int(float(row[9])))
            bbwin_x2.append(int(float(row[10])))
            bbwin_y2.append(int(float(row[11])))
            veh_dist.append(int(round(float(row[12]),2)))

    for idx, val in enumerate(bb_x):
        if ((occlu_v2obj[idx] + occlu_t[idx] + occlu_v2m[idx]) == 0) : #no occlusion at all
            if veh_dist[idx]<150: #restrict to detections at visible range
                xx = 0 #loosen bbox param if needed
                yy =  0 #loosen bbox param if needed
                #cv2.circle(img,(bb_x[idx],bb_y[idx]),5,(0,255,0),-1) #marks veh 2dxy coords
                cv2.rectangle(img, (bbwin_x[idx]-xx, bbwin_y[idx]-yy), (bbwin_x2[idx]+xx, bbwin_y2[idx]+yy), (220, 0, 200), 2)
                cv2.putText(img,str(veh_dist[idx])+"m",(bbwin_x2[idx]+xx-32, bbwin_y[idx]-yy-5),cv2.FONT_HERSHEY_SIMPLEX,0.5,(0,200,0),1,cv2.LINE_AA)
    return img

def plot_peds(img, csv_file, vis_joints):
    pedid = 0 #skip header
    bb_x = []
    bb_y=[]
    bb_x2 = []
    bb_y2 =[]
    joint_type = []
    joint_x = []
    joint_y = []
    count = 0
    for row in csv_file:
        if count == 0:
            count+=1
            continue
        pedid_new = row[2]
        if pedid_new!=pedid:
            bb_x.append(int(float(row[31])))
            bb_y.append(int(float(row[32])))
            bb_x2.append(int(float(row[33])))
            bb_y2.append(int(float(row[34])))
            pedid = pedid_new
        else:
            joint_type.append(int(row[5]))
            joint_x.append(int(float(row[6])))
            joint_y.append(int(float(row[7])))

    for personidx, val in enumerate(bb_x):
        area = abs((bb_x2[personidx]-bb_x[personidx])*(bb_y2[personidx]-bb_y[personidx]))
        if area < 300: #is ped is too small/far skip#
            continue
        cv2.rectangle(img,(bb_x[personidx],bb_y[personidx]),(bb_x2[personidx],bb_y2[personidx]),(0,0,255),2)
        if vis_joints:
            for joint_idx in range(21):
                cv2.circle(img,(joint_x[personidx*21+joint_idx],joint_y[personidx*21+joint_idx]),1,(0,255,0),-1)
                cv2.putText(img, str(joint_idx), (joint_x[personidx*21+joint_idx],joint_y[personidx*21+joint_idx]-2), cv2.FONT_HERSHEY_SIMPLEX, 0.3, (0,200,0), 1, cv2.LINE_AA)
    
    return img
    

