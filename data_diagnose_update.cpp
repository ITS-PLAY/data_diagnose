#include "VspdCTOMySQL.h"
#include <algorithm>
#include <numeric>
#include <vector>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <cstring>
#include <sstream>
#include <math.h>
#include "data_diagnose.h"
#include <sstream>


int main()
{
	char* host="33.134.96.6"; 

	char* user="root"; 

	char* port ="3306";

	char* passwd="Hik_706706"; 

	char* dbname="tongxiang"; 

	char* charset = "utf-8";//支持中文

	char* Msg = "";//消息变量

	char SQL[2048];


	VspdCToMySQL *vspdctomysql = new VspdCToMySQL;

	if(vspdctomysql->ConnMySQL(host,port,dbname,user,passwd,charset,Msg) == 0) 
		printf("数据库连接成功!/n");

	char *split1 = "?@";
	char *split2 = "?";
	char *split3 = "@";


	//状态转移矩阵的建立

	//15分钟流量统计，从tb_flow_lane和tb_flow_direction表中计算
  	string OWNER_CROSS="89";
	string KKBH = "1033";
	vector<string> DIRECTION,LANE_CODE;

	vector<string> LANE_INFO;
	sprintf(SQL,"select DIRECTION,LANE_CODE from tb_lane_trajectory where OWNER_CROSS = '%s' and LANE_NO_REAL >0", OWNER_CROSS.c_str());
    string SP = vspdctomysql->SelectData(SQL,2,Msg);

    char *str = (char *)SP.c_str();
	str = strtok(str,split3);

	while (str != NULL){
		LANE_INFO.push_back(str);
		str = strtok(NULL,split3);  
	}

	for (int i=0; i< LANE_INFO.size(); i++){
		str = (char *)LANE_INFO[i].c_str();
		str = strtok(str,split2);
		DIRECTION.push_back(str);

		str = strtok(NULL,split2);
		LANE_CODE.push_back(str);
	}

	string END_TIME ="2018-01-01 00:00:00";          //时间的初始化
	vector<string> PREVIOUS_DATE;
	string CURRENT_DATE_str;
	vector<string> PREVIOUS_HMS,CURRENT_HMS;
	string HOUR_str="",DATE_str="";
	vector<string> MIN(4);
	MIN[0] = "00";
	MIN[1] = "15";
	MIN[2] = "30";
	MIN[3] = "45";

	for (int i=0; i<24; i++){
		ostringstream oss;
		oss << i;
		HOUR_str = oss.str();
		if (HOUR_str.length() == 1)
			HOUR_str = "0"+HOUR_str;

		//HOUR.push_back(HOUR_str);

		for (int j=0; j<4; j++){
			DATE_str = HOUR_str + ":" + MIN[j] + ":" + "00";
			PREVIOUS_HMS.push_back(DATE_str);            
		}

	}

	int Data_Latency_index =0;
	int Data_Missing_index=0;

	for (int lane_i=0; lane_i <LANE_CODE.size(); lane_i++){

		//所有车道的集合

		//vector<string> LANE_ALL;                                      
		//sprintf(SQL, "select LANE_CODE from tb_lane_trajectory");
		//SP = vspdctomysql->SelectData(SQL,1,Msg);

		//char *str = (char *)SP.c_str();
		//str = strtok(str,split1);
		//while(str != NULL){
		//	LANE_ALL.push_back(str);
		//	str = strtok(NULL,split1);

		//}



		vector<string>::iterator it_date;
		int Lane_Flow=0;


		//计算TB_LANE_TRANSITION_MATRIX表

		//更新历史数据


		for (it_date = PREVIOUS_HMS.begin(); it_date != PREVIOUS_HMS.end(); it_date++){

			//计算流量的阈值区间
			//sprintf(SQL,"SELECT VOLUME,count(*)/(SELECT count(*) FROM `tb_flow_lane` where END_TIME like '%%%%%s' and VOLUME >-1) as FREQUENCY "
			//	     "FROM `tb_flow_lane` where END_TIME like '%%%%%s' and VOLUME >-1 group by VOLUME",(*it_date).c_str());
			//      SP = vspdctomysql->SelectData(SQL,2,Msg);


			sprintf(SQL,"select distinct END_TIME from tb_flow_lane where END_TIME like '%%%%%s' and END_TIME > '%s'",(*it_date).c_str(),END_TIME.c_str());
			SP = vspdctomysql->SelectData(SQL,1,Msg);

			if (SP.length() > 0){
				char *str =(char *)SP.c_str();
				str = strtok(str,split1);

				while(str != NULL){
					PREVIOUS_DATE.push_back(str);
					str = strtok(NULL, split1);
				}

			}

			for (int date_i=0; date_i < PREVIOUS_DATE.size(); date_i++){                  // 日期循环


				string vehicle_pass_table_name="vehicle_pass_info_"+PREVIOUS_DATE[date_i].substr(0,4)+"_"+PREVIOUS_DATE[date_i].substr(5,2)+"_"+PREVIOUS_DATE[date_i].substr(8,2);

				string SP = "";
				char *str = "";
				if (PREVIOUS_DATE[date_i].substr(11, 8) == "00:00:00"){
					string Date_Yesterday = "";
					sprintf(SQL, "select date_add('%s', INTERVAL -1 day)", PREVIOUS_DATE[date_i].c_str());
					SP = vspdctomysql->SelectData(SQL, 1, Msg);
					str = (char *)SP.c_str();
					Date_Yesterday = strtok(str, split1);

					vehicle_pass_table_name = "vehicle_pass_info_" + Date_Yesterday.substr(0, 4) + "_" + Date_Yesterday.substr(5, 2) + "_" + Date_Yesterday.substr(8, 2);
				}




				sprintf(SQL, "select DATE_ADD('%s',INTERVAL 15 MINUTE)",PREVIOUS_DATE[date_i].c_str());      
				SP = vspdctomysql->SelectData(SQL,1,Msg);

				str = (char *)SP.c_str();                                                                //CURRENT_DATE_strr对应的流量查询区间(PREVIOUS_DATE[date_i],CURRENT_DATE_str]
				CURRENT_DATE_str = strtok(str,split1);

				sprintf(SQL, "select DATE_ADD('%s',INTERVAL -15 MINUTE)",PREVIOUS_DATE[date_i].c_str());       //PREVIOUS_DATE对应的流量查询区间(PREVIOUS_DATE_str,PREVIOUS_DATE[date_i]]
				SP = vspdctomysql->SelectData(SQL,1,Msg);

				str = (char *)SP.c_str();
				string PREVIOUS_DATE_str = strtok(str,split1);

				sprintf(SQL,"select SUM(VOLUME_INTERVAL) from tb_flow_lane where LANE_CODE='%s' and END_TIME > '%s' and END_TIME <= '%s'",LANE_CODE[lane_i].c_str(),PREVIOUS_DATE[date_i].c_str(), CURRENT_DATE_str.c_str());
				SP = vspdctomysql->SelectData(SQL,1,Msg);

				str = (char *)SP.c_str();
				str = strtok(str,split2);
				sscanf(str,"%d", &Lane_Flow);                     //车道的当前流量值


				//利用时间差，判断数据延迟的交叉口
				Data_Latency_index = Data_Latency(vspdctomysql, vehicle_pass_table_name, OWNER_CROSS, KKBH, PREVIOUS_DATE[date_i], CURRENT_DATE_str, split2);

				//利用流量法，判断数据缺失的车道
				if (Data_Latency_index == 0){
					Data_Missing_index = Data_Missing(vspdctomysql, OWNER_CROSS, DIRECTION[lane_i], LANE_CODE[lane_i], Lane_Flow, PREVIOUS_DATE[date_i], CURRENT_DATE_str,  split2);

				}else{
					Data_Missing_index = 1;
					sprintf(SQL,"insert into TB_DATA_MISS(LANE_CODE,OWNER_CROSS,DIRECTION,END_TIME,DATA_MISS,TIME_INTERVAL) values('%s','%s','%s','%s',%d,%d)",LANE_CODE[lane_i].c_str(),OWNER_CROSS.c_str(),DIRECTION[lane_i].c_str(),CURRENT_DATE_str.c_str(),Data_Missing_index,Time_Interval_MISS);
					vspdctomysql->InsertData(SQL,Msg);

				}


				//数据缺失情况的更新
				//Data_Missing(vspdctomysql,OWNER_CROSS,DIRECTION,LANE_CODE,PREVIOUS_DATE[date_i],CURRENT_DATE_str, split2);

				//从TB_FLOW_LANE表中，更新TB_LANE_TRANSITION_MATRIX表,先更新，后插入
				//for (LANE_ALL)         //车道循环

				if (Data_Latency_index == 0 & Data_Missing_index == 0){

					sprintf(SQL,"select ID from TB_LANE_TRANSITION_MATRIX where PRE_FLOW= (select SUM(VOLUME_INTERVAL) from tb_flow_lane where END_TIME >'%s' and END_TIME <= '%s' and LANE_CODE ='%s')" 
						"and CUR_FLOW = (select SUM(VOLUME_INTERVAL) from tb_flow_lane where END_TIME >'%s' and END_TIME <= '%s' and LANE_CODE ='%s') and PRE_DATE = '%s'",PREVIOUS_DATE_str.c_str(),PREVIOUS_DATE[date_i].c_str(),LANE_CODE[lane_i].c_str(),PREVIOUS_DATE[date_i].c_str(),CURRENT_DATE_str.c_str(),LANE_CODE[lane_i].c_str(),(*it_date).c_str());
					SP = vspdctomysql->SelectData(SQL,1,Msg);

					if (SP.length() > 0){
						sprintf(SQL,"update TB_LANE_TRANSITION_MATRIX set FREQUENCY=FREQUENCY+1 where PRE_FLOW= (select SUM(VOLUME_INTERVAL) from tb_flow_lane where END_TIME >'%s' and END_TIME <= '%s' and LANE_CODE ='%s')" 
							"and CUR_FLOW = (select SUM(VOLUME_INTERVAL) from tb_flow_lane where END_TIME >'%s' and END_TIME <= '%s' and LANE_CODE ='%s') and PRE_DATE = '%s'",PREVIOUS_DATE_str.c_str(),PREVIOUS_DATE[date_i].c_str(),LANE_CODE[lane_i].c_str(),PREVIOUS_DATE[date_i].c_str(),CURRENT_DATE_str.c_str(),LANE_CODE[lane_i].c_str(),(*it_date).c_str());
						vspdctomysql->UpdateData(SQL,Msg);
					}else{
						sprintf(SQL,"insert into TB_LANE_TRANSITION_MATRIX(LANE_CODE,CUR_FLOW,CUR_DATE,PRE_FLOW,PRE_DATE,FREQUENCY,OWNER_CROSS,DIRECTION)"
							"(select c.*,d.OWNER_CROSS,d.DIRECTION from"
							"   (select a.LANE_CODE,a.VOLUME as CUR_FLOW,DATE_FORMAT(a.END_TIME,'%%%%T') as CUR_DATE,b.VOLUME as PRE_FLOW,DATE_FORMAT(b.END_TIME,'%%%%T') as PRE_DATE,count(*)  from"
							"    (select (case when sum(VOLUME_INTERVAL) is null then 0  else sum(VOLUME_INTERVAL) end) as VOLUME,max(END_TIME) as END_TIME,LANE_CODE from tb_flow_lane where END_TIME >'%s' and END_TIME <= '%s' and LANE_CODE ='%s') a "
							"      LEFT JOIN "
							"       (select (case when sum(VOLUME_INTERVAL) is null then 0  else sum(VOLUME_INTERVAL) end) as VOLUME,max(END_TIME) as END_TIME,LANE_CODE from tb_flow_lane where END_TIME >'%s' and END_TIME <= '%s' and LANE_CODE ='%s') b "
							"       on a.LANE_CODE = b.LANE_CODE) c "
							"LEFT JOIN tb_lane_trajectory d "
							"on c.LANE_CODE = d.LANE_CODE )",PREVIOUS_DATE[date_i].c_str(), CURRENT_DATE_str.c_str(),LANE_CODE[lane_i].c_str(),PREVIOUS_DATE_str.c_str(),PREVIOUS_DATE[date_i].c_str(),LANE_CODE[lane_i].c_str());            //以车道、日期为变量，后续加入END_TIME限制更新最新数据
						vspdctomysql->InsertData(SQL,Msg);
					}


					if (date_i > 0){    

						sprintf(SQL, "select DATE_ADD('%s',INTERVAL -1 DAY)",PREVIOUS_DATE[date_i].c_str());   //计算前一天相同时刻的日期
						SP = vspdctomysql->SelectData(SQL,1,Msg);

						str = (char *)SP.c_str();
						string PRE_DAY_DATE  = strtok(str,split1);    

						sprintf(SQL, "select DATE_ADD('%s',INTERVAL -15 MINUTE)",PRE_DAY_DATE.c_str());  //PRE_DAY_DATE对应的流量查询区间(PRE_DAY_DATE_str,PRE_DAY_DATE]
						SP = vspdctomysql->SelectData(SQL,1,Msg);

						str = (char *)SP.c_str();
						string PRE_DAY_DATE_str = strtok(str, split1);

						//for (OWNER_CROSS,DIRECTION)    //进口道循环

						//从TB_FLOW_DIRECTION表中，更新TB_DIRECTION_TRANSITION_MATRIX表,先更新，后插入
						sprintf(SQL, "select ID from TB_DIRECTION_TRANSITION_MATRIX where PRE_FLOW_AVE= (select round(SUM(q1.VOLUME)/q1.LANE_NUM_EFFE) as VOLUME from (select SUM(VOLUME_INTERVAL) as VOLUME,SUM(LANE_NUM_EFFE) as LANE_NUM_EFFE from tb_flow_direction where END_TIME >'%s' and END_TIME <= '%s' and OWNER_CROSS=%s and DIRECTION=%s and VOLUME >= 0 group by END_TIME) q1)"
							" and CUR_FLOW_AVE = (select round(SUM(q1.VOLUME)/q1.LANE_NUM_EFFE) as VOLUME from (select SUM(VOLUME_INTERVAL) as VOLUME,SUM(LANE_NUM_EFFE) as LANE_NUM_EFFE from tb_flow_direction where END_TIME >'%s' and END_TIME <= '%s' and OWNER_CROSS=%s and DIRECTION=%s and VOLUME >= 0 group by END_TIME) q1) and PRE_DATE = '%s'",PRE_DAY_DATE_str.c_str(),PRE_DAY_DATE.c_str(),OWNER_CROSS.c_str(),DIRECTION[lane_i].c_str(),PREVIOUS_DATE_str.c_str(),PREVIOUS_DATE[date_i].c_str(),OWNER_CROSS.c_str(),DIRECTION[lane_i].c_str(),(*it_date).c_str());
						SP = vspdctomysql->SelectData(SQL,1,Msg);

						if (SP.length() > 0){
							sprintf(SQL,"update TB_DIRECTION_TRANSITION_MATRIX set FREQUENCY=FREQUENCY+1 where PRE_FLOW_AVE= (select round(SUM(q1.VOLUME)/q1.LANE_NUM_EFFE) as VOLUME from (select SUM(VOLUME_INTERVAL) as VOLUME,SUM(LANE_NUM_EFFE) as LANE_NUM_EFFE from tb_flow_direction where END_TIME >'%s' and END_TIME <= '%s' and OWNER_CROSS=49 and DIRECTION=0 and VOLUME >= 0 group by END_TIME) q1)"
								" and CUR_FLOW_AVE = (select round(SUM(q1.VOLUME)/q1.LANE_NUM_EFFE) as VOLUME from (select SUM(VOLUME_INTERVAL) as VOLUME,SUM(LANE_NUM_EFFE) as LANE_NUM_EFFE from tb_flow_direction where END_TIME >'%s' and END_TIME <= '%s' and OWNER_CROSS=%s and DIRECTION=%s and VOLUME >= 0 group by END_TIME) q1) and PRE_DATE = '%s'",PRE_DAY_DATE_str.c_str(),PRE_DAY_DATE.c_str(),OWNER_CROSS.c_str(),DIRECTION[lane_i].c_str(),PREVIOUS_DATE_str.c_str(),PREVIOUS_DATE[date_i].c_str(),OWNER_CROSS.c_str(),DIRECTION[lane_i].c_str(),(*it_date).c_str());
							vspdctomysql->UpdateData(SQL,Msg);
						}else{

							sprintf(SQL,"insert into TB_DIRECTION_TRANSITION_MATRIX(OWNER_CROSS,DIRECTION,LANE_NUM_EFFE,LANE_NUM_SUM,PRE_FLOW_AVE,PRE_DATE,CUR_FLOW_AVE,CUR_DATE,FREQUENCY) "
								"(select a.OWNER_CROSS,a.DIRECTION,a.LANE_NUM_EFFE,a.LANE_NUM_SUM,a.VOLUME as PRE_FLOW_AVE,DATE_FORMAT(a.END_TIME,'%%%%T') as PRE_DATE,b.VOLUME as CUR_FLOW_AVE,DATE_FORMAT(b.END_TIME,'%%%%T') as CUR_DATE,count(*) from "
								" (select x1.OWNER_CROSS,x1.DIRECTION,c1.LANE_NUM_EFFE,c1.LANE_NUM_SUM,x1.VOLUME,x1.END_TIME from "
								"(select round(SUM(q1.VOLUME)/q1.LANE_NUM_EFFE) as VOLUME,max(q1.END_TIME) as END_TIME,q1.OWNER_CROSS,q1.DIRECTION from "
								"(select SUM(VOLUME_INTERVAL) as VOLUME,SUM(LANE_NUM_EFFE) as LANE_NUM_EFFE,OWNER_CROSS,DIRECTION,END_TIME from tb_flow_direction where END_TIME >'%s' and END_TIME <= '%s' and OWNER_CROSS=%s and DIRECTION=%s and VOLUME >= 0 group by END_TIME) q1) x1 "
								"left join "
								"(select a1.OWNER_CROSS,a1.DIRECTION,a1.LANE_NUM_SUM as LANE_NUM_SUM,count(LANE_NO_REAL) as LANE_NUM_EFFE from tb_lane_trajectory b1 inner join "
								"(select OWNER_CROSS,DIRECTION,count(LANE_NO) as LANE_NUM_SUM from tb_lane_trajectory where OWNER_CROSS = %s and DIRECTION=%s) a1 "
								"on b1.OWNER_CROSS=a1.OWNER_CROSS and b1.DIRECTION = a1.DIRECTION where b1.LANE_NO_REAL >0)c1 "
								"on x1.OWNER_CROSS=c1.OWNER_CROSS and x1.DIRECTION = c1.DIRECTION) a "
								" LEFT JOIN "
								" (select round(SUM(q1.VOLUME)/q1.LANE_NUM_EFFE) as VOLUME,max(q1.END_TIME) as END_TIME,q1.OWNER_CROSS,q1.DIRECTION from "
								"(select SUM(VOLUME_INTERVAL) as VOLUME,SUM(LANE_NUM_EFFE) as LANE_NUM_EFFE,OWNER_CROSS,DIRECTION,END_TIME from tb_flow_direction where END_TIME >'%s' and END_TIME <= '%s' and OWNER_CROSS=%s and DIRECTION=%s and VOLUME >= 0 group by END_TIME) q1) b "
								" on a.OWNER_CROSS = b.OWNER_CROSS and a.DIRECTION = b.DIRECTION having a.OWNER_CROSS is not null)",PRE_DAY_DATE_str.c_str(),PRE_DAY_DATE.c_str(),OWNER_CROSS.c_str(),DIRECTION[lane_i].c_str(),OWNER_CROSS.c_str(),DIRECTION[lane_i].c_str(),PREVIOUS_DATE_str.c_str(),PREVIOUS_DATE[date_i].c_str(),OWNER_CROSS.c_str(),DIRECTION[lane_i].c_str());         //以交叉口编号和方向编号、日期为变量，后续加入END_TIME限制更新最新数据
							vspdctomysql->InsertData(SQL,Msg);

						}
					}

					//车道分配比例计算
					sprintf(SQL," insert into TB_LANE_FLOW_RATIO(OWNER_CROSS,DIRECTION,LANE_NUM_EFFE,LANE_NUM_SUM,LANE_CODE,END_TIME,DIRECTION_FLOW,LANE_FLOW,LANE_FLOW_RATIO) "
						"(select c.OWNER_CROSS,c.DIRECTION,c.LANE_NUM_EFFE,c.LANE_NUM_SUM,f.LANE_CODE,c.END_TIME,c.D_VOLUME,f.L_VOLUME,(case when c.D_VOLUME>0 then f.L_VOLUME/c.D_VOLUME when c.D_VOLUME=0 then 0 end) RATIO from "
						"(select x1.OWNER_CROSS,x1.DIRECTION,d1.LANE_NUM_EFFE,d1.LANE_NUM_SUM,x1.VOLUME as D_VOLUME,x1.END_TIME from "
						"(select sum(VOLUME_INTERVAL) as VOLUME,max(END_TIME) as END_TIME,OWNER_CROSS,DIRECTION from tb_flow_direction where END_TIME >'%s' and END_TIME <= '%s' and OWNER_CROSS=%s and DIRECTION=%s and VOLUME >= 0 ) x1 "
						"left join "
						"(select c1.OWNER_CROSS,c1.DIRECTION,c1.LANE_NUM_SUM as LANE_NUM_SUM,count(LANE_NO_REAL) as LANE_NUM_EFFE from tb_lane_trajectory d1 inner join "
						"(select a1.OWNER_CROSS,a1.DIRECTION,count(a1.LANE_NO) as LANE_NUM_SUM from tb_lane_trajectory a1 inner join "
						"(select OWNER_CROSS,DIRECTION from tb_lane_trajectory where LANE_CODE='%s')b1 "
						"on a1.OWNER_CROSS=b1.OWNER_CROSS and a1.DIRECTION = b1.DIRECTION) c1 "
						"on d1.OWNER_CROSS=c1.OWNER_CROSS and d1.DIRECTION = c1.DIRECTION where d1.LANE_NO_REAL >0)d1 "
						"on x1.OWNER_CROSS=d1.OWNER_CROSS and x1.DIRECTION = d1.DIRECTION) c "
						"right join "
						"(select e.OWNER_CROSS,e.DIRECTION,e.LANE_CODE,sum(d.VOLUME_INTERVAL) as L_VOLUME,max(d.END_TIME) as END_TIME from tb_flow_lane d "
						"right join "
						"(select OWNER_CROSS,DIRECTION,LANE_CODE from tb_lane_trajectory where LANE_CODE='%s') e "
						"on d.LANE_CODE = e.LANE_CODE "
						"where END_TIME >'%s' and END_TIME <= '%s') f "
						"on c.OWNER_CROSS = f.OWNER_CROSS and c.DIRECTION = f.DIRECTION and c.END_TIME = f.END_TIME)",PREVIOUS_DATE_str.c_str(),PREVIOUS_DATE[date_i].c_str(),OWNER_CROSS.c_str(),DIRECTION[lane_i].c_str(),LANE_CODE[lane_i].c_str(),LANE_CODE[lane_i].c_str(),PREVIOUS_DATE_str.c_str(),PREVIOUS_DATE[date_i].c_str());      //以车道为变量，后续加入END_TIME限制更新最新数据

					vspdctomysql->InsertData(SQL,Msg);  


				}


			}

			PREVIOUS_DATE.swap(vector<string>());
		}


		//根据阈值法，计算流量的阈值区间
		vector<string> Flow_str_temp;
		Flow_Diagram Flow_Observe_Sequence;
		float Lane_Flow_Freq=0.0;

		int Sample_Num=0;

		string date_temp="2018-10-16 ";
		string it_before = "",it_before_one = "",it_before_two = "";
		for (it_date = PREVIOUS_HMS.begin(); it_date != PREVIOUS_HMS.end(); it_date++){

			//      it_before = it_date-PREVIOUS_HMS.begin();      //将5分钟采集的流量累加为15分钟的流量数据
			//it_before_one = ((it_before-1)+96)%96;
			//it_before_two = ((it_before-2)+96)%96;

			it_before = date_temp + (*it_date);
			sprintf(SQL, "select date_format(date_add('%s', INTERVAL -5 MINUTE),'%%%%%T')",it_before.c_str());
			SP = vspdctomysql->SelectData(SQL,1,Msg);
			str = (char *)SP.c_str();
			it_before_one = strtok(str,split1);

			sprintf(SQL, "select date_format(date_add('%s', INTERVAL -10 MINUTE),'%%%%%T')",it_before.c_str());
			SP = vspdctomysql->SelectData(SQL,1,Msg);
			str = (char *)SP.c_str();
			it_before_two = strtok(str,split1);

			sprintf(SQL,"SELECT count(*) from "
				"(SELECT DATE_FORMAT(END_TIME,'%%%%%Y-%%%%%m-%%%%%D') as date_time FROM `tb_flow_lane` where (END_TIME like '%%%%%s' or END_TIME like '%%%%%s' or END_TIME like '%%%%%s') and LANE_CODE='%s' group by date_time) a",(*it_date).c_str(),it_before_one.c_str(),it_before_two.c_str(),LANE_CODE[lane_i].c_str());
			SP = vspdctomysql->SelectData(SQL,1,Msg);

			str = (char *)SP.c_str();
			str = strtok(str,split1);
			sscanf(str,"%d",&Sample_Num);


			sprintf(SQL,"insert into TB_FLOW_FREQUENCY_SA(OWNER_CROSS,DIRECTION,LANE_CODE,END_DATE,FLOW_VALUE,FLOW_FREQUENCY,SAMPLE_NUM) (select a.OWNER_CROSS,a.DIRECTION,a.LANE_CODE,'%s' as END_DATE,a.VOLUME,count(*)/%d as FLOW_FREQUENCY, %d as Sample_Num "
				"from (SELECT OWNER_CROSS,DIRECTION,LANE_CODE,SUM(VOLUME_INTERVAL) as VOLUME,DATE_FORMAT(END_TIME,'%%%%%Y-%%%%%m-%%%%%D') as date_time FROM `tb_flow_lane` where (END_TIME like '%%%%%s' or END_TIME like '%%%%%s' or END_TIME like '%%%%%s') and LANE_CODE='%s' group by date_time) a group by a.VOLUME) ",(*it_date).c_str(), Sample_Num,Sample_Num, (*it_date).c_str(), it_before_one.c_str(), it_before_two.c_str(), LANE_CODE[lane_i].c_str());
			vspdctomysql->InsertData(SQL,Msg);


			sprintf(SQL,"select a.VOLUME,count(*)/%d "
				"from (SELECT SUM(VOLUME_INTERVAL) as VOLUME,DATE_FORMAT(END_TIME,'%%%%%Y-%%%%%m-%%%%%D') as date_time FROM `tb_flow_lane` where (END_TIME like '%%%%%s' or END_TIME like '%%%%%s' or END_TIME like '%%%%%s') and LANE_CODE='%s' group by date_time) a group by a.VOLUME ",Sample_Num,(*it_date).c_str(),it_before_one.c_str(),it_before_two.c_str(),LANE_CODE[lane_i].c_str());
			SP = vspdctomysql->SelectData(SQL,2,Msg);

			str = (char *)SP.c_str();
			str = strtok(str,split3);

			while(str != NULL){
				Flow_str_temp.push_back(str);
				str = strtok(NULL,split3);

			}

			for(int i=0; i < Flow_str_temp.size(); i++){
				str = (char *)(Flow_str_temp[i]).c_str();
				str = strtok(str,split2);

				sscanf(str,"%d",&Lane_Flow);
				Flow_Observe_Sequence.FLOW_VALUE.push_back(Lane_Flow);

				str = strtok(NULL,split2);
				sscanf(str,"%f",&Lane_Flow_Freq);
				Flow_Observe_Sequence.FLOW_FREQUENCY.push_back(Lane_Flow_Freq);

			}

			EM_Output Flow_EM_output;
			MLM(&Flow_EM_output,Flow_Observe_Sequence);

			sprintf(SQL, "insert into TB_FLOW_LIMIT_SA(LANE_CODE,OWNER_CROSS,DIRECTION,END_DATE,FLOW_AVE,FLOW_VAR,SAMPLE_NUM) values('%s','%s','%s','%s',%f,%f,%d)",LANE_CODE[lane_i].c_str(),OWNER_CROSS.c_str(),DIRECTION[lane_i].c_str(),(*it_date).c_str(),Flow_EM_output.EM_ave,Flow_EM_output.EM_sd,Time_Interval_ANOMALY);
			vspdctomysql->InsertData(SQL,Msg);

			Flow_str_temp.swap(vector<string>());
			Flow_Observe_Sequence.FLOW_VALUE.swap(vector<int>());
			Flow_Observe_Sequence.FLOW_FREQUENCY.swap(vector<float>());

		}

	}

}


int Data_Latency(VspdCToMySQL *vspdctomysql,string vehicle_pass_table_name,string OWNER_CROSS, string KKBH, string date_start, string date_end,char *split2)       //数据延迟
{
	char SQL[1024];
	char* Msg = "";//消息变量
	int data_latency =0,vehicle_num_latency=0,vehicle_num_sum=0;
	float data_latency_ratio=0.0,data_latency_time=0.0;

	sprintf(SQL, "SELECT count(*) as num,AVG(UNIX_TIMESTAMP(CREATE_DATE)-UNIX_TIMESTAMP(JGSK_FORMATE))/60 FROM tx_device.`%s` where KKBH='%s' and CREATE_DATE between '%s' and '%s' "
		"and (UNIX_TIMESTAMP(CREATE_DATE)-UNIX_TIMESTAMP(JGSK_FORMATE))>%d having num >0", vehicle_pass_table_name.c_str(),KKBH.c_str(),date_start.c_str(),date_end.c_str(),Time_Limit_Value*60);
	string SP = vspdctomysql->SelectData(SQL,2,Msg);

	if(SP.length() > 0){

		char *str = (char *)SP.c_str();
		str = strtok(str,split2);
		sscanf(str,"%d",&vehicle_num_latency);

		str = strtok(NULL,split2);
		sscanf(str,"%f",&data_latency_time);

		sprintf(SQL, "SELECT count(*) FROM tx_device.`%s` where KKBH='%s' and CREATE_DATE between '%s' and '%s'", vehicle_pass_table_name.c_str(),KKBH.c_str(),date_start.c_str(),date_end.c_str());
		string SP = vspdctomysql->SelectData(SQL,1,Msg);

		if (SP.length()>0){
			str = (char *)SP.c_str();
			str = strtok(str,split2);
			sscanf(str,"%d",&vehicle_num_sum);

			data_latency_ratio = vehicle_num_latency*1.0/vehicle_num_sum;

			if (data_latency_ratio >= data_latency_ratio_Limit){
				data_latency = 1;
			}

			sprintf(SQL,"insert into TB_DATA_LATENCY(OWNER_CROSS,END_TIME,DATA_LATENCY,DATA_LATENCY_TIME,TIME_INTERVAL) values('%s','%s','%d','%f',%d)",OWNER_CROSS.c_str(),date_end.c_str(),data_latency,data_latency_time,Time_Interval_LATENCY);
			vspdctomysql->InsertData(SQL,Msg);
		}

	}else
	{
		//将总体的延迟时间取平均
		sprintf(SQL, "SELECT (case when count(*)>0 then AVG(UNIX_TIMESTAMP(CREATE_DATE)-UNIX_TIMESTAMP(JGSK_FORMATE))/60 else 0.0 end) FROM tx_device.`%s` where KKBH='%s' and CREATE_DATE between '%s' and '%s'", vehicle_pass_table_name.c_str(),KKBH.c_str(),date_start.c_str(),date_end.c_str());
		SP = vspdctomysql->SelectData(SQL,1,Msg);

		if (SP.length() > 0){
			char *str = (char *)SP.c_str();
			str = strtok(str, split2);

			sscanf(str, "%f", &data_latency_time);

		}
		else{
			data_latency_time = 0.0;
		}

		sprintf(SQL, "insert into TB_DATA_LATENCY(OWNER_CROSS,END_TIME,DATA_LATENCY,DATA_LATENCY_TIME,TIME_INTERVAL) values('%s','%s','%d','%f',%d)", OWNER_CROSS.c_str(), date_end.c_str(), data_latency, data_latency_time, Time_Interval_LATENCY);
		vspdctomysql->InsertData(SQL, Msg);
	}

	return data_latency;

}


int Data_Missing(VspdCToMySQL *vspdctomysql, string OWNER_CROSS, string DIRECTION, string LANE_CODE, int Lane_Flow, string END_TIME_downrange, string END_TIME,  char *split2)
{
	char SQL[1024];
	char *Msg="";
	string SP="";
	char *str ="";

	int Data_Missing_index =0;

	if (Lane_Flow == 0){
		string END_TIME_HM = END_TIME.substr(11,5);

		if (END_TIME_HM>"06:00" & END_TIME_HM <= "23:59"){
			sprintf(SQL, "select date_add('%s',INTERVAL -1 HOUR)",END_TIME.c_str());
			SP = vspdctomysql->SelectData(SQL,1,Msg);

			str =(char *)SP.c_str();
			END_TIME_downrange = strtok(str,split2);
		}else{
			sprintf(SQL, "select date_add('%s',INTERVAL -3 HOUR)",END_TIME.c_str());
			SP = vspdctomysql->SelectData(SQL,1,Msg);

			str =(char *)SP.c_str();
			END_TIME_downrange = strtok(str,split2);
		}


		sprintf(SQL,"select SUM(VOLUME) from tb_flow_lane where LANE_CODE='%s' and END_TIME > '%s' and END_TIME <= '%s'",LANE_CODE.c_str(),END_TIME_downrange.c_str(),END_TIME.c_str());
		SP = vspdctomysql->SelectData(SQL,1,Msg);

		char *str = (char *)SP.c_str();
		str = strtok(str,split2);

		sscanf(str,"%d", &Lane_Flow);

		if (Lane_Flow == 0){
			Data_Missing_index = 1;
		}
	}

	//已存在某一END_TIME，则更新DATA_MISS；否则插入新的记录
	sprintf(SQL,"update TB_DATA_MISS set DATA_MISS = %d where END_TIME in "
		"(select c.END_TIME from (select b.END_TIME from TB_DATA_MISS a right join "
		"(select END_TIME from TB_FLOW_LANE where END_TIME > '%s' and END_TIME <= '%s' and LANE_CODE='%s') b "
		"on a.END_TIME= b.END_TIME "
		"having b.END_TIME is not null) c)",Data_Missing_index,END_TIME_downrange.c_str(),END_TIME.c_str(),LANE_CODE.c_str());
	vspdctomysql->UpdateData(SQL,Msg);

	sprintf(SQL,"insert into TB_DATA_MISS(LANE_CODE,OWNER_CROSS,DIRECTION,END_TIME,DATA_MISS,TIME_INTERVAL) values('%s','%s','%s','%s',%d,%d)",LANE_CODE.c_str(),OWNER_CROSS.c_str(),DIRECTION.c_str(),END_TIME.c_str(),Data_Missing_index,Time_Interval_MISS);
	vspdctomysql->InsertData(SQL,Msg);

	return Data_Missing_index;

}


void MLM(EM_Output *Flow_EM_output,Flow_Diagram Flow_Predict)
{
	EM_Output MLM_result;
	float volume_ave=0.0,volume_sd=0.0;
	for (int i =0; i < Flow_Predict.FLOW_VALUE.size(); i++){
		volume_ave += Flow_Predict.FLOW_VALUE[i] * Flow_Predict.FLOW_FREQUENCY[i];
	}

	for (int i =0; i < Flow_Predict.FLOW_VALUE.size(); i++){
		volume_sd += Flow_Predict.FLOW_FREQUENCY[i] *pow((Flow_Predict.FLOW_VALUE[i]-volume_ave),2);
	}

	Flow_EM_output->EM_ave = volume_ave;
	Flow_EM_output->EM_sd = sqrt(volume_sd);

}


