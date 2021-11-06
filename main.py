from plot_funcs import *

def main(f_dir, num_of_img_idx, vis_jnt):
    idx_to_plot = list(range(int(num_of_img_idx)))
    for i in idx_to_plot:
        try:
            ped_csv_file = csv.reader(open(f_dir+"coords_ped"+str(i)+".csv"))
            veh_csv_file = csv.reader(open(f_dir+"coords_veh"+str(i)+".csv"))
            img = cv2.imread(f_dir+"image_"+str(i)+".jpg")
            rows,cols,_ = img.shape
            print("Plotting for: {}\n".format(f_dir+"image_"+str(i)+".jpg"))
            img_plot_ped = plot_peds(img,ped_csv_file, vis_joints=vis_jnt)
            final_img = plot_veh(img,veh_csv_file)
            cv2.putText(final_img, "image_"+str(i)+".jpg" , (5,20), cv2.FONT_HERSHEY_SIMPLEX, 0.8, (0,0,0), 2, cv2.LINE_AA)
            cv2.putText(final_img, "https://github.com/Ivan-LZY/GTD-mod" , (cols-394,rows-8), cv2.FONT_HERSHEY_SIMPLEX, 0.6, (80,80,80), 2, cv2.LINE_AA)
            cv2.imshow("Press ESC to exit. Press SPACEBAR to save image in current directory.", final_img)
            k = cv2.waitKey(0)
            if k==27: #esc to exit#
                cv2.destroyAllWindows()
                break
            if k == 32: #spacebar to save image in current dir#
                cv2.imwrite("image_"+str(i)+"_overlay.png", final_img)
        except:
            print("Unable to open files for {}. Please check your directories again.".format(i))
 
if __name__== "__main__":
    main("./cam_current/", 1000, True) 
    

