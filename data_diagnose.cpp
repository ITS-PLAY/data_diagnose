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
#include <sstream>
#include <windows.h>
#include "data_diagnose.h"


int main()
{
	char* host="33.134.96.6"; 
	//char* host="10.0.40.125";

	char* user="root"; 

	char* port ="3306";

	char* passwd="Hik_706706"; 

	char* dbname="tongxiang"; 

	char* charset = "utf-8";//֧������

	char* Msg = "";        //��Ϣ����

	char SQL[2048];


	VspdCToMySQL *vspdctomysql = new VspdCToMySQL;

	if(vspdctomysql->ConnMySQL(host,port,dbname,user,passwd,charset,Msg) == 0) 
		printf("���ݿ����ӳɹ�!/n");


	/*
    string date_start =Get_Date();
	int MIN = 0;

	while (date_start.length() != 0){

		sprintf(SQL,"SELECT MINUTE('%s')",date_start.c_str());
		string SP = vspdctomysql->SelectData(SQL,1,Msg);

		char *str = (char *)SP.c_str();
		string MINUTE = strtok(str,"?@");

		sscanf(MINUTE.c_str(),"%d",&MIN);
		MIN = MIN % 15;

		sprintf(SQL,"SELECT SECOND('%s')",date_start.c_str());
		SP = vspdctomysql->SelectData(SQL,1,Msg);

		str = (char *)SP.c_str();
		string SECOND = strtok(str,"?@");

		if (MIN ==0 & SECOND == "0"){
			printf("�����쳣��Ϲ����� %s ��ʼִ��;\n",date_start.c_str());

			Sleep(90000);
			DATA_DIAGNOSE(vspdctomysql,date_start);

			date_start =Get_Date();
			printf("�� %s ִ�н���.\n",date_start.c_str());
			cout<<endl;

		}

		date_start =Get_Date();

	} 
    */

	string date_start = "2018-11-27 09:15:00";
	DATA_DIAGNOSE(vspdctomysql, date_start);

	vspdctomysql->CloseMySQLConn();
	delete vspdctomysql;
	vspdctomysql = NULL;

	return 0;

}


void DATA_DIAGNOSE(VspdCToMySQL *vspdctomysql, string Date_Nowday)
{
	char* Msg = "";        //��Ϣ����
	char SQL[2048];

	char *split1 = "?@";
	char *split2 = "?";
	char *split3 = "@";

	vector<string> CROSS_SET,KKBH_SET;

	CROSS_SET.push_back("114");
	CROSS_SET.push_back("21");
	CROSS_SET.push_back("89");
	CROSS_SET.push_back("49");


	KKBH_SET.push_back("1030");
	KKBH_SET.push_back("1006");
	KKBH_SET.push_back("1033");
	KKBH_SET.push_back("1015");


	string vehicle_pass_table_name="vehicle_pass_info_"+Date_Nowday.substr(0,4)+"_"+Date_Nowday.substr(5,2)+"_"+Date_Nowday.substr(8,2);

	string SP = "";
	char *str = "";
	if (Date_Nowday.substr(11, 8) == "00:00:00"){
		string Date_Yesterday = "";
		sprintf(SQL, "select date_add('%s', INTERVAL -1 day)", Date_Nowday.c_str());
		SP = vspdctomysql->SelectData(SQL, 1, Msg);
		str = (char *)SP.c_str();
		Date_Yesterday = strtok(str, split1);

		vehicle_pass_table_name = "vehicle_pass_info_" + Date_Yesterday.substr(0, 4) + "_" + Date_Yesterday.substr(5, 2) + "_" + Date_Yesterday.substr(8, 2);
	}


	string CURRENT_DATE_str;
	sprintf(SQL, "select DATE_ADD('%s',INTERVAL -15 MINUTE)",Date_Nowday.c_str());      
	SP = vspdctomysql->SelectData(SQL,1,Msg);

	str = (char *)SP.c_str();                                                                //CURRENT_DATE_str��Ӧ��������ѯ����(PREVIOUS_DATE[date_i],CURRENT_DATE_str]
	CURRENT_DATE_str = strtok(str,split1);

	sprintf(SQL, "select DATE_ADD('%s',INTERVAL -15 MINUTE)",CURRENT_DATE_str.c_str());       //PREVIOUS_DATE��Ӧ��������ѯ����(PREVIOUS_DATE_str,PREVIOUS_DATE[date_i]]
	SP = vspdctomysql->SelectData(SQL,1,Msg);

	str = (char *)SP.c_str();
	string PREVIOUS_DATE_str = strtok(str,split1);


	vector<string> DIRECTION,LANE_CODE;
	vector<string> LANE_INFO;

	int Data_Latency_index =0;
	int Data_Missing_index=0;
	int Data_Anomaly_Index=0;


	for (int cross_i=0; cross_i < CROSS_SET.size(); cross_i++){

		if (cross_i < (CROSS_SET.size()-1)){
			sprintf(SQL,"select DIRECTION,LANE_CODE from tb_lane_trajectory where OWNER_CROSS = '%s' and LANE_NO_REAL >0", CROSS_SET[cross_i].c_str());
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
		}else{
			DIRECTION.push_back("0");
			LANE_CODE.push_back("4902");
		}



		//����ʱ���ж������ӳٵĽ����
		Data_Latency_index = Data_Latency(vspdctomysql, vehicle_pass_table_name, CROSS_SET[cross_i], KKBH_SET[cross_i], CURRENT_DATE_str, Date_Nowday, split2);


	    for (int lane_i=0; lane_i <LANE_CODE.size(); lane_i++){

			//15��������ͳ�ƣ���tb_flow_lane��tb_flow_direction���м���
			//string OWNER_CROSS="49",DIRECTION="0",LANE_CODE = "4902";
			//string KKBH="1015";
			//string Date_Nowday ="2018-10-25 19:30:00";          //ʱ��ĳ�ʼ��
			//vector<string> PREVIOUS_DATE;

			//���г����ļ���

			//vector<string> LANE_ALL;                                      
			//sprintf(SQL, "select LANE_CODE from tb_lane_trajectory");
			//string SP = vspdctomysql->SelectData(SQL,1,Msg);

			//char *str = (char *)SP.c_str();
			//str = strtok(str,split1);
			//while(str != NULL){
			//	LANE_ALL.push_back(str);
			//	str = strtok(NULL,split1);

			//}

			int Lane_Flow=0,lane_flow=0;

			sprintf(SQL,"select SUM(VOLUME_INTERVAL) from tb_flow_lane where LANE_CODE='%s' and END_TIME > '%s' and END_TIME <= '%s'",LANE_CODE[lane_i].c_str(),CURRENT_DATE_str.c_str(),Date_Nowday.c_str());
			SP = vspdctomysql->SelectData(SQL,1,Msg);

			str = (char *)SP.c_str();
			str = strtok(str,split2);
			sscanf(str,"%d", &Lane_Flow);                     //�����ĵ�ǰ����ֵ


			//һ������Ʒ�ģ�͵ļ���

			string Date_Nowday_downrange="",END_TIME="";
			Flow_Diagram Lane_Flow_Iteration;
			int Lane_Flow_Nowday=Lane_Flow;

			for (int pred_i=0; pred_i < Predictor_Data_Num; pred_i++){

				if (pred_i ==0){
					sprintf(SQL,"select date_add('%s', INTERVAL 15 MINUTE)",Date_Nowday.c_str());
					string SP = vspdctomysql->SelectData(SQL,1,Msg);
					char *str = (char *)SP.c_str();
					END_TIME = strtok(str,split1);

					Lane_Flow_Iteration.FLOW_VALUE.push_back(Lane_Flow_Nowday);
					Lane_Flow_Iteration.FLOW_FREQUENCY.push_back(1.0);

					Lane_Flow_Iteration = Data_Anomaly_predict(vspdctomysql,CROSS_SET[cross_i],DIRECTION[lane_i],LANE_CODE[lane_i],Date_Nowday,Lane_Flow_Iteration,split1,split2,split3);


					//��������Ļ�ϸ�˹�ֲ�����
					EM_Output Flow_EM_output;
					//Expectation_Maximization(&Flow_EM_output,Lane_Flow_Iteration);
					MLM(&Flow_EM_output,Lane_Flow_Iteration);

					if (Flow_EM_output.EM_ave == 0.0 & Flow_EM_output.EM_sd == 0.0){

					}else{
						sprintf(SQL, "insert into TB_FLOW_LIMIT_HMM(OWNER_CROSS,LANE_CODE,DIRECTION,END_TIME,FLOW_AVE,FLOW_VAR,TIME_INTERVAL) values('%s','%s','%s','%s',%f,%f,%d)",CROSS_SET[cross_i].c_str(),LANE_CODE[lane_i].c_str(),DIRECTION[lane_i].c_str(),END_TIME.c_str(),Flow_EM_output.EM_ave,Flow_EM_output.EM_sd,Time_Interval_ANOMALY);
						vspdctomysql->InsertData(SQL,Msg);
					}

					sprintf(SQL,"update TB_FLOW_LIMIT_HMM set FLOW_REAL=%d where LANE_CODE='%s' and END_TIME='%s'",Lane_Flow_Nowday,LANE_CODE[lane_i].c_str(),Date_Nowday.c_str());
					vspdctomysql->UpdateData(SQL,Msg);

				}else{

					sprintf(SQL,"select date_add('%s', INTERVAL %d MINUTE)",Date_Nowday.c_str(),15*(pred_i+1));
					string SP = vspdctomysql->SelectData(SQL,1,Msg);
					char *str = (char *)SP.c_str();
					END_TIME = strtok(str,split1);

					sprintf(SQL,"select date_add('%s',INTERVAL %d MINUTE)",Date_Nowday.c_str(),15*pred_i);
					SP = vspdctomysql->SelectData(SQL,1,Msg);

					str = (char *)SP.c_str();
					Date_Nowday = strtok(str,split1);

					Lane_Flow_Iteration = Data_Anomaly_predict(vspdctomysql,CROSS_SET[cross_i],DIRECTION[lane_i],LANE_CODE[lane_i],Date_Nowday,Lane_Flow_Iteration,split1,split2,split3);

					//��������Ļ�ϸ�˹�ֲ�����
					EM_Output Flow_EM_output;
					//Expectation_Maximization(&Flow_EM_output,Lane_Flow_Iteration);
					MLM(&Flow_EM_output,Lane_Flow_Iteration);

					sprintf(SQL, "insert into TB_FLOW_LIMIT_HMM(OWNER_CROSS,LANE_CODE,DIRECTION,END_TIME,FLOW_AVE,FLOW_VAR,TIME_INTERVAL) values('%s','%s','%s','%s',%f,%f,%d)",CROSS_SET[cross_i].c_str(),LANE_CODE[lane_i].c_str(),DIRECTION[lane_i].c_str(),END_TIME.c_str(),Flow_EM_output.EM_ave,Flow_EM_output.EM_sd,Time_Interval_ANOMALY);
					vspdctomysql->InsertData(SQL,Msg);

				}

			} 


			Lane_Flow_Iteration.FLOW_FREQUENCY.swap(vector<float>());
			Lane_Flow_Iteration.FLOW_VALUE.swap(vector<int>());


			//�������������ж�����ȱʧ�ĳ���
			if (Data_Latency_index == 0){
			    Data_Missing_index = Data_Missing(vspdctomysql, CROSS_SET[cross_i], DIRECTION[lane_i], LANE_CODE[lane_i], Lane_Flow, CURRENT_DATE_str,  Date_Nowday,  split2);

				//����Markovģ�ͣ��ж������쳣�ĳ���
				if (Data_Missing_index == 0){

					//���û���Markovģ�͵��������ж������쳣�ĳ���
					sprintf(SQL,"select ID from TB_FLOW_LIMIT_HMM where LANE_CODE='%s' and END_TIME='%s'",LANE_CODE[lane_i].c_str(),Date_Nowday.c_str());
					SP = vspdctomysql->SelectData(SQL,1,Msg);

					if (SP.length() >0){
						Data_Anomaly_HMM_judge(vspdctomysql,CROSS_SET[cross_i],DIRECTION[lane_i],LANE_CODE[lane_i], Lane_Flow_Nowday, CURRENT_DATE_str,  Date_Nowday,  split2);
					}else{
						Data_Anomaly_SA_judge(vspdctomysql,CROSS_SET[cross_i],DIRECTION[lane_i],LANE_CODE[lane_i], Lane_Flow_Nowday, CURRENT_DATE_str,  Date_Nowday,  split2);
					}

				}else{
					Data_Anomaly_Index = 1;

				   sprintf(SQL,"insert into TB_DATA_ANOMALY(LANE_CODE,OWNER_CROSS,DIRECTION,END_TIME,DATA_ANOMALY,TIME_INTERVAL) values('%s','%s','%s','%s',%d,%d)",LANE_CODE[lane_i].c_str(),CROSS_SET[cross_i].c_str(),DIRECTION[lane_i].c_str(),Date_Nowday.c_str(),Data_Anomaly_Index,Time_Interval_ANOMALY);
				   vspdctomysql->InsertData(SQL,Msg);

				}
			}else{
				Data_Missing_index = 1;
				sprintf(SQL,"insert into TB_DATA_MISS(LANE_CODE,OWNER_CROSS,DIRECTION,END_TIME,DATA_MISS,TIME_INTERVAL) values('%s','%s','%s','%s',%d,%d)",LANE_CODE[lane_i].c_str(),CROSS_SET[cross_i].c_str(),DIRECTION[lane_i].c_str(),Date_Nowday.c_str(),Data_Missing_index,Time_Interval_MISS);
				vspdctomysql->InsertData(SQL,Msg);

				Data_Anomaly_Index = 1;

				sprintf(SQL,"insert into TB_DATA_ANOMALY(LANE_CODE,OWNER_CROSS,DIRECTION,END_TIME,DATA_ANOMALY,TIME_INTERVAL) values('%s','%s','%s','%s',%d,%d)",LANE_CODE[lane_i].c_str(),CROSS_SET[cross_i].c_str(),DIRECTION[lane_i].c_str(),Date_Nowday.c_str(),Data_Anomaly_Index,Time_Interval_ANOMALY);
				vspdctomysql->InsertData(SQL,Msg);
			}

			/*
			if (Data_Latency_index == 0 & Data_Missing_index == 0){
				//������ֵ���ĸ��ʷֲ���TB_FLOW_FREQUENCY_SA

				float lane_flow_freq=0.0;
				vector<string> Flow_str_temp;
				Flow_Diagram Flow_Observe_Sequence;


				sprintf(SQL,"update TB_FLOW_FREQUENCY_SA set FLOW_FREQUENCY=(case when FLOW_VALUE=%d then  (FLOW_FREQUENCY+round(1/SAMPLE_NUM,4))*round(SAMPLE_NUM/(SAMPLE_NUM+1),4) "
					"else  FLOW_FREQUENCY*round(SAMPLE_NUM/(SAMPLE_NUM+1),4) end), SAMPLE_NUM=SAMPLE_NUM+1  where END_DATE=date_format('%s','%%%%T') and LANE_CODE='%s'",lane_flow,Date_Nowday.c_str(),LANE_CODE[lane_i].c_str());
				vspdctomysql->UpdateData(SQL,Msg);         //���������ĸ��ʷֲ�

				sprintf(SQL, "select ID from TB_FLOW_FREQUENCY_SA where FLOW_VALUE=%d and END_DATE=date_format('%s','%%%%T') and LANE_CODE='%s'",lane_flow,Date_Nowday.c_str(),LANE_CODE[lane_i].c_str());
				SP = vspdctomysql->SelectData(SQL,1,Msg);

				if (SP.length() == 0){
					int Sample_Num=0;
					sprintf(SQL, "select SAMPLE_NUM from TB_FLOW_FREQUENCY_SA where END_DATE=date_format('%s','%%%%T') and LANE_CODE='%s' limit 1",Date_Nowday.c_str(),LANE_CODE[lane_i].c_str());
					SP = vspdctomysql->SelectData(SQL,1,Msg);

					str = (char *)SP.c_str();
					str = strtok(str,split1);
					sscanf(str,"%d",&Sample_Num);

					sprintf(SQL, "insert into TB_FLOW_FREQUENCY_SA(LANE_CODE,OWNER_CROSS,DIRECTION,END_DATE,FLOW_VALUE,FLOW_FREQUENCY,SAMPLE_NUM) values('%s','%s','%s',date_format('%s','%%%%T'),%d,%f,%d)",LANE_CODE[lane_i].c_str(),CROSS_SET[cross_i].c_str(),DIRECTION[lane_i].c_str(),Date_Nowday.c_str(),lane_flow,1.0/Sample_Num,Sample_Num);
					vspdctomysql->InsertData(SQL,Msg);    //���û�ж�Ӧ�������������ݲ��뵽TB_FLOW_FREQUENCY_SA��
				}

				sprintf(SQL,"select FLOW_VALUE,FLOW_FREQUENCY from TB_FLOW_FREQUENCY_SA where LANE_CODE='%s' and END_DATE=date_format('%s','%%%%T')",LANE_CODE[lane_i].c_str(),Date_Nowday.c_str());
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

					sscanf(str,"%d",&lane_flow);
					Flow_Observe_Sequence.FLOW_VALUE.push_back(lane_flow);

					str = strtok(NULL,split2);
					sscanf(str,"%f",&lane_flow_freq);
					Flow_Observe_Sequence.FLOW_FREQUENCY.push_back(lane_flow_freq);

				}

				EM_Output Flow_EM_output;
				MLM(&Flow_EM_output,Flow_Observe_Sequence);

				sprintf(SQL, "update TB_FLOW_LIMIT_SA set FLOW_AVE=%f, FLOW_VAR=%f where LANE_CODE='%s' and END_DATE=date_format('%s','%%%%T')",Flow_EM_output.EM_ave,Flow_EM_output.EM_sd,LANE_CODE[lane_i].c_str(),Date_Nowday.c_str());
				vspdctomysql->UpdateData(SQL,Msg);    //ʵʱ����TB_FLOW_LIMIT_SA���е�������ֵ�ͷ���

				Flow_str_temp.swap(vector<string>());
				Flow_Observe_Sequence.FLOW_VALUE.swap(vector<int>());
				Flow_Observe_Sequence.FLOW_FREQUENCY.swap(vector<float>());


				//////�������µ����ڸ��£�����ת�ƾ����

				//��TB_FLOW_LANE���У�����TB_LANE_TRANSITION_MATRIX��,�ȸ��£������

				//for (LANE_ALL)         //����ѭ��
				sprintf(SQL,"select ID from TB_LANE_TRANSITION_MATRIX where PRE_FLOW= (select SUM(VOLUME_INTERVAL) from tb_flow_lane where END_TIME >'%s' and END_TIME <= '%s' and LANE_CODE ='%s')" 
					"and CUR_FLOW = (select SUM(VOLUME_INTERVAL) from tb_flow_lane where END_TIME >'%s' and END_TIME <= '%s' and LANE_CODE ='%s') and CUR_DATE = (select DATE_FORMAT('%s','%%%%T'))",CURRENT_DATE_str.c_str(),Date_Nowday.c_str(),LANE_CODE[lane_i].c_str(),PREVIOUS_DATE_str.c_str(),CURRENT_DATE_str.c_str(),LANE_CODE[lane_i].c_str(),Date_Nowday.c_str());
				SP = vspdctomysql->SelectData(SQL,1,Msg);

				if (SP.length() > 0){
					sprintf(SQL,"update TB_LANE_TRANSITION_MATRIX set FREQUENCY=FREQUENCY+1 where PRE_FLOW= (select SUM(VOLUME_INTERVAL) from tb_flow_lane where END_TIME >'%s' and END_TIME <= '%s' and LANE_CODE ='%s')" 
						"and CUR_FLOW = (select SUM(VOLUME_INTERVAL) from tb_flow_lane where END_TIME >'%s' and END_TIME <= '%s' and LANE_CODE ='%s') and CUR_DATE = (select DATE_FORMAT('%s','%%%%T'))",CURRENT_DATE_str.c_str(),Date_Nowday.c_str(),LANE_CODE[lane_i].c_str(),PREVIOUS_DATE_str.c_str(),CURRENT_DATE_str.c_str(),LANE_CODE[lane_i].c_str(),Date_Nowday.c_str());
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
						"on c.LANE_CODE = d.LANE_CODE )", CURRENT_DATE_str.c_str(),Date_Nowday.c_str(),LANE_CODE[lane_i].c_str(),PREVIOUS_DATE_str.c_str(),CURRENT_DATE_str.c_str(),LANE_CODE[lane_i].c_str());            //�Գ���������Ϊ��������������END_TIME���Ƹ�����������
					vspdctomysql->InsertData(SQL,Msg);
				}


				//��TB_FLOW_DIRECTION���У�����TB_DIRECTION_TRANSITION_MATRIX��,�ȸ��£������
				sprintf(SQL, "select DATE_ADD('%s',INTERVAL -1 DAY)",Date_Nowday.c_str());   //����ǰһ����ͬʱ�̵�����
				SP = vspdctomysql->SelectData(SQL,1,Msg);

				str = (char *)SP.c_str();
				string PRE_DAY_DATE  = strtok(str,split1);    

				sprintf(SQL, "select DATE_ADD('%s',INTERVAL -15 MINUTE)",PRE_DAY_DATE.c_str());  //PRE_DAY_DATE��Ӧ��������ѯ����(PRE_DAY_DATE_str,PRE_DAY_DATE]
				SP = vspdctomysql->SelectData(SQL,1,Msg);

				str = (char *)SP.c_str();
				string PRE_DAY_DATE_str = strtok(str, split1);

				//for (OWNER_CROSS,DIRECTION)    //���ڵ�ѭ��
				sprintf(SQL, "select ID from TB_DIRECTION_TRANSITION_MATRIX where PRE_FLOW_AVE= (select round(SUM(q1.VOLUME)/q1.LANE_NUM_EFFE) as VOLUME from (select SUM(VOLUME_INTERVAL) as VOLUME,SUM(LANE_NUM_EFFE) as LANE_NUM_EFFE from tb_flow_direction where END_TIME >'%s' and END_TIME <= '%s' and OWNER_CROSS='%s' and DIRECTION='%s' and VOLUME >= 0 group by END_TIME) q1)"
					" and CUR_FLOW_AVE = (select round(SUM(q1.VOLUME)/q1.LANE_NUM_EFFE) as VOLUME from (select SUM(VOLUME_INTERVAL) as VOLUME,SUM(LANE_NUM_EFFE) as LANE_NUM_EFFE from tb_flow_direction where END_TIME >'%s' and END_TIME <= '%s' and OWNER_CROSS='%s' and DIRECTION='%s' and VOLUME >= 0 group by END_TIME) q1) and CUR_DATE = (select DATE_FORMAT('%s','%%%%T'))",PRE_DAY_DATE_str.c_str(),PRE_DAY_DATE.c_str(),CROSS_SET[cross_i].c_str(),DIRECTION[lane_i].c_str(),CURRENT_DATE_str.c_str(),Date_Nowday.c_str(),CROSS_SET[cross_i].c_str(),DIRECTION[lane_i].c_str(),Date_Nowday.c_str());
				SP = vspdctomysql->SelectData(SQL,1,Msg);

				if (SP.length() > 0){
					sprintf(SQL,"update TB_DIRECTION_TRANSITION_MATRIX set FREQUENCY=FREQUENCY+1 where PRE_FLOW_AVE= (select round(SUM(q1.VOLUME)/q1.LANE_NUM_EFFE) as VOLUME from (select SUM(VOLUME_INTERVAL) as VOLUME,SUM(LANE_NUM_EFFE) as LANE_NUM_EFFE from tb_flow_direction where END_TIME >'%s' and END_TIME <= '%s' and OWNER_CROSS=%s and DIRECTION=%s and VOLUME >= 0 group by END_TIME) q1)"
						" and CUR_FLOW_AVE = (select round(SUM(q1.VOLUME)/q1.LANE_NUM_EFFE) as VOLUME from (select SUM(VOLUME_INTERVAL) as VOLUME,SUM(LANE_NUM_EFFE) as LANE_NUM_EFFE from tb_flow_direction where END_TIME >'%s' and END_TIME <= '%s' and OWNER_CROSS=%s and DIRECTION=%s and VOLUME >= 0 group by END_TIME) q1) and CUR_DATE = (select DATE_FORMAT('%s','%%%%T'))",PRE_DAY_DATE_str.c_str(),PRE_DAY_DATE.c_str(),CROSS_SET[cross_i].c_str(),DIRECTION[lane_i].c_str(),CURRENT_DATE_str.c_str(),Date_Nowday.c_str(),CROSS_SET[cross_i].c_str(),DIRECTION[lane_i].c_str(),Date_Nowday.c_str());
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
						" on a.OWNER_CROSS = b.OWNER_CROSS and a.DIRECTION = b.DIRECTION having a.OWNER_CROSS is not null)",PRE_DAY_DATE_str.c_str(),PRE_DAY_DATE.c_str(),CROSS_SET[cross_i].c_str(),DIRECTION[lane_i].c_str(),CROSS_SET[cross_i].c_str(),DIRECTION[lane_i].c_str(),CURRENT_DATE_str.c_str(),Date_Nowday.c_str(),CROSS_SET[cross_i].c_str(),DIRECTION[lane_i].c_str());         //�Խ���ڱ�źͷ����š�����Ϊ��������������END_TIME���Ƹ�����������
					vspdctomysql->InsertData(SQL,Msg);

				}

				//���������������
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
					"on c.OWNER_CROSS = f.OWNER_CROSS and c.DIRECTION = f.DIRECTION and c.END_TIME = f.END_TIME)",CURRENT_DATE_str.c_str(),Date_Nowday.c_str(),CROSS_SET[cross_i].c_str(),DIRECTION[lane_i].c_str(),LANE_CODE[lane_i].c_str(),LANE_CODE[lane_i].c_str(),CURRENT_DATE_str.c_str(),Date_Nowday.c_str());      //�Գ���Ϊ��������������END_TIME���Ƹ�����������

				vspdctomysql->InsertData(SQL,Msg);  
			}

			*/


        }
		LANE_INFO.swap(vector<string>());
		DIRECTION.swap(vector<string>());
		LANE_CODE.swap(vector<string>());
	}

}





Flow_Diagram Data_Anomaly_predict(VspdCToMySQL *vspdctomysql, string OWNER_CROSS,string DIRECTION, string LANE_CODE, string Date_Nowday,Flow_Diagram Lane_Flow_Iteration, char *split1, char *split2, char *split3)   //�����쳣
{
	char SQL[1024];
	char* Msg="";

	string Date_Nowday_downrange= "";      //��ȡ��ǰʱ�̡���ǰʱ�̵�ͳ����߽� �Լ� ƽ����������
	string Date_Nowday_Nexttime = "";                //Ԥ���ʱ��
	vector<string> SP_string;
	int Lane_Flow_Yesterday=0;                          //�����У�����ʵ������ֵ����
	//int Lane_Flow_Nowday=0;

	Flow_Diagram Flow_Direction_Sequence,Flow_State_Sequence,Flow_State_Sequence_Temp,Flow_Observe_SampleData,Flow_Observe_Sequence;   //״̬���к͹۲����еĶ���

	//Flow_Diagram Flow_Predict_Distr;

	//��ǰʱ�̵�ͳ����߽�
	sprintf(SQL,"select date_add('%s', INTERVAL -15 MINUTE)",Date_Nowday.c_str());
	string SP = vspdctomysql->SelectData(SQL,1,Msg);
	char *str = (char *)SP.c_str();
	Date_Nowday_downrange = strtok(str,split1);

	//���㵱ǰ����һ��ͳ��ʱ��
	sprintf(SQL,"select date_add('%s', INTERVAL 15 MINUTE)",Date_Nowday.c_str());
	SP = vspdctomysql->SelectData(SQL,1,Msg);
	str = (char *)SP.c_str();
	Date_Nowday_Nexttime = strtok(str,split1);

	//������һʱ�����ڣ�ͳ��ǰһ�����ʵƽ����������
	string Date_Yesterday = "",Date_Yesterday_downrange = "";
	sprintf(SQL, "select date_add('%s', INTERVAL -1 day)",Date_Nowday_Nexttime.c_str());
	SP = vspdctomysql->SelectData(SQL,1,Msg);
	str = (char *)SP.c_str();
	Date_Yesterday = strtok(str,split1);

	sprintf(SQL, "select date_add('%s', INTERVAL -15 MINUTE)",Date_Yesterday.c_str());
	SP = vspdctomysql->SelectData(SQL,1,Msg);
	str = (char *)SP.c_str();
	Date_Yesterday_downrange = strtok(str,split1);

	//������һʱ�����ڣ�ͳ������ͬһʱ�̵���ʵƽ����������
	string Date_Same_Week = "", Date_Same_Week_downrange = "";
	sprintf(SQL, "select date_add('%s', INTERVAL -7 day)",Date_Nowday_Nexttime.c_str());
    SP = vspdctomysql->SelectData(SQL,1,Msg);
	str = (char *)SP.c_str();
	Date_Same_Week = strtok(str,split1);

	sprintf(SQL, "select date_add('%s', INTERVAL -15 MINUTE)",Date_Same_Week.c_str());
	SP = vspdctomysql->SelectData(SQL,1,Msg);
	str = (char *)SP.c_str();
	Date_Same_Week_downrange = strtok(str,split1);

	string Lane_Flow_String = "";   //�洢״̬���е���ʷƽ�����ڵ�����
	sprintf(SQL, "select round(SUM(q1.VOLUME)/q1.LANE_NUM_EFFE) from (select SUM(VOLUME_INTERVAL) as VOLUME,SUM(LANE_NUM_EFFE) as LANE_NUM_EFFE from tb_flow_direction where "
		"END_TIME >'%s' and END_TIME <= '%s' and OWNER_CROSS='%s' and DIRECTION='%s' and VOLUME_INTERVAL >= 0 group by END_TIME) q1",Date_Yesterday_downrange.c_str(),Date_Yesterday.c_str(),OWNER_CROSS.c_str(),DIRECTION.c_str());
	SP = vspdctomysql->SelectData(SQL,1,Msg);
	str = (char *)SP.c_str();
	str = strtok(str,split1);
    Lane_Flow_String = str;                      //����Ľ��ڵ�����
	//sscanf(str,"%d",&Lane_Flow_Yesterday);    //����Ľ��ڵ�����

	sprintf(SQL, "select round(SUM(q1.VOLUME)/q1.LANE_NUM_EFFE) from (select SUM(VOLUME_INTERVAL) as VOLUME,SUM(LANE_NUM_EFFE) as LANE_NUM_EFFE from tb_flow_direction where "
		"END_TIME >'%s' and END_TIME <= '%s' and OWNER_CROSS='%s' and DIRECTION='%s' and VOLUME_INTERVAL >= 0 group by END_TIME) q1",Date_Same_Week_downrange.c_str(),Date_Same_Week.c_str(),OWNER_CROSS.c_str(),DIRECTION.c_str());
	SP = vspdctomysql->SelectData(SQL,1,Msg);
	str = (char *)SP.c_str();
	str = strtok(str,split1);
	Lane_Flow_String = Lane_Flow_String + "," + str;                      //����ͬһʱ�̵Ľ��ڵ�����


	//1)���Բ��֣� ״̬ת�����е�����

	sprintf(SQL, "select CUR_FLOW_AVE*LANE_NUM_EFFE,FREQUENCY/(select SUM(FREQUENCY) from TB_DIRECTION_TRANSITION_MATRIX where PRE_DATE = date_format('%s','%%%%T') and PRE_FLOW_AVE in (%s)) "
		"from TB_DIRECTION_TRANSITION_MATRIX where PRE_DATE = date_format('%s','%%%%T') and PRE_FLOW_AVE in (%s)",Date_Nowday_Nexttime.c_str(),Lane_Flow_String.c_str(),Date_Nowday_Nexttime.c_str(),Lane_Flow_String.c_str());  //����
	SP = vspdctomysql->SelectData(SQL,2,Msg);

	if (SP.length() == 0){
		sprintf(SQL,"SELECT PRE_FLOW_AVE FROM tb_direction_transition_matrix where PRE_DATE=date_format('%s','%%%%T') order by abs(PRE_FLOW_AVE-%d) limit 1",Date_Nowday_Nexttime.c_str(),Lane_Flow_Yesterday);
		SP =vspdctomysql->SelectData(SQL,1,Msg);
		str = (char *)SP.c_str();
		str = strtok(str,split1);
		sscanf(str,"%d",&Lane_Flow_Yesterday);

		sprintf(SQL, "select CUR_FLOW_AVE*LANE_NUM_EFFE,FREQUENCY/(select SUM(FREQUENCY) from TB_DIRECTION_TRANSITION_MATRIX where PRE_DATE = date_format('%s','%%%%T') and PRE_FLOW_AVE in (%s)) "
			"from TB_DIRECTION_TRANSITION_MATRIX where PRE_DATE = date_format('%s','%%%%T') and PRE_FLOW_AVE in (%s)",Date_Nowday_Nexttime.c_str(),Lane_Flow_String.c_str(),Date_Nowday_Nexttime.c_str(),Lane_Flow_String.c_str());  //����
		SP = vspdctomysql->SelectData(SQL,2,Msg);
	}

	if (SP.length() > 0){

		str = (char *)SP.c_str();
		str = strtok(str,split3);
		while(str != NULL){
			SP_string.push_back(str);
			str = strtok(NULL, split3);
		}

		int flow_int = 0;
		float freq_float = 0.0;
		vector<int>::iterator it_dir_flow;
		int it_dir_num=0;

		for (int i=0; i < SP_string.size(); i++){
			str = (char *)(SP_string[i]).c_str();
			str = strtok(str,split2);
			sscanf(str,"%d",&flow_int);

			str = strtok(NULL,split2);
			sscanf(str,"%f",&freq_float);

			it_dir_flow = find(Flow_Direction_Sequence.FLOW_VALUE.begin(), Flow_Direction_Sequence.FLOW_VALUE.end(), flow_int);

			if (it_dir_flow != Flow_Direction_Sequence.FLOW_VALUE.end()){
				it_dir_num = it_dir_flow - Flow_Direction_Sequence.FLOW_VALUE.begin();
                Flow_Direction_Sequence.FLOW_FREQUENCY[it_dir_num] = Flow_Direction_Sequence.FLOW_FREQUENCY[it_dir_num] + freq_float;
			}else{
				Flow_Direction_Sequence.FLOW_VALUE.push_back(flow_int);
				Flow_Direction_Sequence.FLOW_FREQUENCY.push_back(freq_float);
			}


		}
		SP_string.swap(vector<string>());


		//2�����Բ��֣����복�������������
		string HM_Last="";
		float lane_flow_ratio =0.0;
		sprintf(SQL, "SELECT date_format('%s','%%%%T')",Date_Nowday_Nexttime.c_str());
		SP = vspdctomysql->SelectData(SQL,1,Msg);
		str = (char *)SP.c_str();
		HM_Last = strtok(str,split1);

		//��û�ж�Ӧ�Ľ��ڵ�����ʱ�������Ȳ���������������
		sprintf(SQL,"SELECT LANE_FLOW_RATIO from TB_LANE_FLOW_RATIO where LANE_CODE='%s' and END_TIME like '%%%%%s' order by END_TIME desc LIMIT 1",LANE_CODE.c_str(),HM_Last.c_str());
		SP = vspdctomysql->SelectData(SQL,1,Msg);
		str = (char *)SP.c_str();
		str = strtok(str,split1);
		sscanf(str,"%f",&lane_flow_ratio); 


		for (int i=0; i<Flow_Direction_Sequence.FLOW_VALUE.size(); i++){                //���ݽ��ڵ�����ת���õ��ĳ������� ��Ϊ ����ֲ�

			int lane_flow = 0;
			int it_num = 0;

			sprintf(SQL,"SELECT LANE_FLOW,count(*)/(select count(*) from TB_LANE_FLOW_RATIO where LANE_CODE='%s' and END_TIME like '%%%%%s' and DIRECTION_FLOW=%d) "
				"FROM TB_LANE_FLOW_RATIO where LANE_CODE='%s' and END_TIME like '%%%%%s' and DIRECTION_FLOW=%d group by LANE_FLOW",LANE_CODE.c_str(),HM_Last.c_str(),Flow_Direction_Sequence.FLOW_VALUE[i],LANE_CODE.c_str(),HM_Last.c_str(),Flow_Direction_Sequence.FLOW_VALUE[i]);
			SP = vspdctomysql->SelectData(SQL,2,Msg);

			if (SP.length() > 0){
				str = (char *)SP.c_str();
				str = strtok(str,split3);
				while(str != NULL){
					SP_string.push_back(str);
					str = strtok(NULL, split3);
				}

				for (int j=0; j<SP_string.size(); j++){
					str = (char *)SP_string[j].c_str();
					str = strtok(str,split2);
					sscanf(str,"%d",&flow_int);

					str = strtok(NULL,split2);
					sscanf(str,"%f",&freq_float);

					vector<int>::iterator it_flow;
                    it_flow = find(Flow_State_Sequence.FLOW_VALUE.begin(),Flow_State_Sequence.FLOW_VALUE.end(),flow_int);

					if (it_flow != Flow_State_Sequence.FLOW_VALUE.end()){
						it_num= it_flow - Flow_State_Sequence.FLOW_VALUE.begin();
						Flow_State_Sequence.FLOW_FREQUENCY[it_num] = Flow_State_Sequence.FLOW_FREQUENCY[it_num] + Flow_Direction_Sequence.FLOW_FREQUENCY[i] * freq_float ;
					}else{
						Flow_State_Sequence.FLOW_VALUE.push_back(flow_int);
						Flow_State_Sequence.FLOW_FREQUENCY.push_back(Flow_Direction_Sequence.FLOW_FREQUENCY[i]);

						Flow_Observe_Sequence.FLOW_VALUE.push_back(flow_int);    //�۲����к�״̬���о�����ͬ��ȡֵ����
						Flow_Observe_Sequence.FLOW_FREQUENCY.push_back(0.0);
					}

				}

				SP_string.swap(vector<string>());

			}else{
				lane_flow = (int)Flow_Direction_Sequence.FLOW_VALUE[i] * lane_flow_ratio;
				vector<int>::iterator it_flow;
				/*for (int i=0; i<Flow_State_Sequence.FLOW_VALUE.size(); i++){*/
				it_flow = find(Flow_State_Sequence.FLOW_VALUE.begin(),Flow_State_Sequence.FLOW_VALUE.end(),lane_flow);
				if (it_flow != Flow_State_Sequence.FLOW_VALUE.end()){
					it_num = it_flow - Flow_State_Sequence.FLOW_VALUE.begin();
					Flow_State_Sequence.FLOW_FREQUENCY[it_num] = Flow_State_Sequence.FLOW_FREQUENCY[it_num] + Flow_Direction_Sequence.FLOW_FREQUENCY[i] ;

				}else{
					Flow_State_Sequence.FLOW_VALUE.push_back(lane_flow);
					Flow_State_Sequence.FLOW_FREQUENCY.push_back(Flow_Direction_Sequence.FLOW_FREQUENCY[i]);

					Flow_Observe_Sequence.FLOW_VALUE.push_back(lane_flow);    //�۲����к�״̬���о�����ͬ��ȡֵ����
					Flow_Observe_Sequence.FLOW_FREQUENCY.push_back(0.0);
				}
				//}  
			}

		}


		//����Flow_State_Sequence��ֵ
        Flow_State_Sequence_Temp.FLOW_FREQUENCY.assign(Flow_State_Sequence.FLOW_FREQUENCY.begin(),Flow_State_Sequence.FLOW_FREQUENCY.end());

		//3�����Բ��֣��۲����е�����

 		int Sample_state_num = 0;                  //��¼����ֲ�����Ч��������

		for (int obflow_i=0; obflow_i < Lane_Flow_Iteration.FLOW_VALUE.size(); obflow_i++){   //��Ԥ���ʱ�䳤�ȴ���15����ʱ��������һʱ�����������зֲ�

			for (int k=0; k<Flow_State_Sequence.FLOW_VALUE.size(); k++){       //��״̬�����е������ֲ���Ϊ����ֲ���TB_LANE_TRANSITION_MATRIX���е������Էֲ���Ϊ �������ݣ����������ĺ���ֲ�
				sprintf(SQL, "select PRE_FLOW,FREQUENCY/(select SUM(FREQUENCY) from TB_LANE_TRANSITION_MATRIX where CUR_DATE=date_format('%s','%%%%T') and CUR_FLOW=%d and LANE_CODE='%s') from TB_LANE_TRANSITION_MATRIX where CUR_DATE=date_format('%s','%%%%T') and CUR_FLOW=%d and LANE_CODE='%s'",Date_Nowday.c_str(),Flow_State_Sequence.FLOW_VALUE[k],LANE_CODE.c_str(),Date_Nowday.c_str(),Flow_State_Sequence.FLOW_VALUE[k],LANE_CODE.c_str());
				SP = vspdctomysql->SelectData(SQL,2,Msg);

				if (SP.length() > 0){
					str = (char *)SP.c_str();
					str = strtok(str,split3);
					while(str != NULL){
						SP_string.push_back(str);
						str = strtok(NULL, split3);
					}

					for (int i=0; i < SP_string.size(); i++){
						str = (char *)(SP_string[i]).c_str();
						str = strtok(str,split2);
						sscanf(str,"%d",&flow_int);

						Flow_Observe_SampleData.FLOW_VALUE.push_back(flow_int);       

						str = strtok(NULL,split2);
						sscanf(str,"%f",&freq_float);

						Flow_Observe_SampleData.FLOW_FREQUENCY.push_back(freq_float);

					}

					//Ԥ�⵱��Ľ��ڵ�ƽ����������
					vector<int>::iterator it_observe;
					int num_observe = 0;


					it_observe = find(Flow_Observe_SampleData.FLOW_VALUE.begin(),Flow_Observe_SampleData.FLOW_VALUE.end(),Lane_Flow_Iteration.FLOW_VALUE[obflow_i]);       //ͨ���۲����е��������ݣ����������ֲ�
					if (it_observe != Flow_Observe_SampleData.FLOW_VALUE.end()){
						num_observe = it_observe - Flow_Observe_SampleData.FLOW_VALUE.begin();
						float fre_predict = Flow_State_Sequence.FLOW_FREQUENCY[k] * Flow_Observe_SampleData.FLOW_FREQUENCY[num_observe];          //���ʸ���

						Flow_State_Sequence.FLOW_FREQUENCY[k] = fre_predict;    
						//Sample_state_num ++ ;

					}else{
						Flow_State_Sequence.FLOW_FREQUENCY[k] = 0.0;
					}

					SP_string.swap(vector<string>());
					Flow_Observe_SampleData.FLOW_VALUE.swap(vector<int>());
					Flow_Observe_SampleData.FLOW_FREQUENCY.swap(vector<float>());

				}       //if (SP.length() > 0){
				else{
					Flow_State_Sequence.FLOW_FREQUENCY[k] = 0.0;
				}
			}           //for (int k=0; k<Flow_State_Sequence.FLOW_VALUE.size(); k++){ 

			if (Flow_State_Sequence.FLOW_VALUE.size() >= 1){      //����۲����еĸ���
				float suum=accumulate(Flow_State_Sequence.FLOW_FREQUENCY.begin(),Flow_State_Sequence.FLOW_FREQUENCY.end(),0.0); 

				if (suum > 0.0){
					for (int k=0; k<Flow_State_Sequence.FLOW_FREQUENCY.size(); k++){
						Flow_Observe_Sequence.FLOW_FREQUENCY[k] = Flow_Observe_Sequence.FLOW_FREQUENCY[k] + Flow_State_Sequence.FLOW_FREQUENCY[k] * Lane_Flow_Iteration.FLOW_FREQUENCY[obflow_i] / suum;
					}
				}else{

				}

			}

			//��Flow_State_Sequence���¸���ԭ����ֵ
            Flow_State_Sequence.FLOW_FREQUENCY.swap(Flow_State_Sequence_Temp.FLOW_FREQUENCY);

		}   //for (int obflow_i=0; obflow_i < Lane_Flow_Iteration.FLOW_VALUE.size(); obflow_i++){



		if (Flow_Observe_Sequence.FLOW_VALUE.size() >= 3){      //����۲��������������ľ�ֵ�ͷ���
			float suum=accumulate(Flow_Observe_Sequence.FLOW_FREQUENCY.begin(),Flow_Observe_Sequence.FLOW_FREQUENCY.end(),0.0); 

			if (suum > 0.0){
				for (int k=0; k<Flow_Observe_Sequence.FLOW_FREQUENCY.size(); k++){

					Flow_Observe_Sequence.FLOW_FREQUENCY[k] = Flow_Observe_Sequence.FLOW_FREQUENCY[k]/suum;

					if (Flow_Observe_Sequence.FLOW_FREQUENCY[k] > 0.0)
						Sample_state_num ++;
				}
			}else{
				for (int k=0; k<Flow_Observe_Sequence.FLOW_FREQUENCY.size(); k++){

					if (Flow_Observe_Sequence.FLOW_FREQUENCY[k] > 0.0)
						Sample_state_num ++;
				}
			}

		}else{
			Flow_Observe_Sequence.FLOW_VALUE.swap(vector<int>());
			Flow_Observe_Sequence.FLOW_FREQUENCY.swap(vector<float>());
            
			Flow_Observe_Sequence.FLOW_VALUE.push_back(0);
			Flow_Observe_Sequence.FLOW_FREQUENCY.push_back(0.0);

            return Flow_Observe_Sequence;
		}

		if (Sample_state_num <3){     //������������3��ʱ��ֱ�Ӷ�����ȡ���ȷֲ�

			for (int k=0; k<Flow_Observe_Sequence.FLOW_FREQUENCY.size(); k++){
				Flow_Observe_Sequence.FLOW_FREQUENCY[k] = 1.0/Flow_Observe_Sequence.FLOW_FREQUENCY.size();
			}
		}

		return Flow_Observe_Sequence;


	}else {

		//��15���Ӻ󣬽�����������������������µ�TB_DIRECTION_TRANSITION_MATRIX��

		//���ݹ۲����е����������Ƶ�
		Flow_Observe_Sequence.FLOW_VALUE.swap(vector<int>());
		Flow_Observe_Sequence.FLOW_FREQUENCY.swap(vector<float>());

		Flow_Observe_Sequence.FLOW_VALUE.push_back(0);
		Flow_Observe_Sequence.FLOW_FREQUENCY.push_back(0.0);

		return Flow_Observe_Sequence;

	}



}


void Expectation_Maximization(EM_Output *Flow_EM_output,Flow_Diagram Flow_Predict)
{

	//EM_Output EM_result;

	//��������

	int Origin_Sample = 0, Final_Sample = 0;
	int num = 0;
	vector<int> Sample_Flow;
	int flow_max= *max_element(Flow_Predict.FLOW_VALUE.begin(),Flow_Predict.FLOW_VALUE.end());
	int flow_min= *min_element(Flow_Predict.FLOW_VALUE.begin(),Flow_Predict.FLOW_VALUE.end());
	Origin_Sample = (flow_max-flow_min) * Gaussian_Num;

	for (int i=0; i<Flow_Predict.FLOW_VALUE.size(); i++){
		num = int(Flow_Predict.FLOW_FREQUENCY[i] * Origin_Sample + 0.5);   //ĳһ������Ӧ����������С��λʱ����������
		Final_Sample = Final_Sample + num;
		for (int j=0; j <num; j++){
			Sample_Flow.push_back(Flow_Predict.FLOW_VALUE[i]);
		}
	}

	//������ʼ��
	vector<EM_Output> EM_AVE_SD(Gaussian_Num);  //��˹�ֲ��Ĳ������� 

	float Weight[Gaussian_Num], Weight_temp[Gaussian_Num]={0.0};   //��ϸ�˹�ֲ���ϵ��
	vector<vector<float> > Gama(Final_Sample,vector<float>(Gaussian_Num,0.0)),Gama_temp(Final_Sample, vector<float>(Gaussian_Num,0.0));      //������Ӧ�ȵĳ�ʼ��


	for (int i=0; i<Gaussian_Num; i++){                  //��ʼ��Ȩ��ϵ��
		Weight[i] = 1*1.0/Gaussian_Num;                 
	}

	int Sum_Sample = 0, index = 0;
	for (int si = 1; si <= Final_Sample; si++){       //��ʼ����ֵ�ͷ���
		Sum_Sample += Sample_Flow[si-1];
		if (si%(Final_Sample/Gaussian_Num) == 0){
			index = si/(Final_Sample/Gaussian_Num);
			EM_AVE_SD[index-1].EM_ave = 1.0*Sum_Sample/si;
			EM_AVE_SD[index-1].EM_sd = 1.0;
		}
	}


	float Pro_SUM = 0.0;
	float Likelihood_result_before =0.0, Likelihood_result_next = 100;
	int Cal_Times = 10000;

	//������ʼ
	//E�������ݵ�ǰģ�Ͳ���������ÿ�����ݵ�xi����ÿ����k�ĸ��ʣ�����ֲ�����������֪��

	while (Cal_Times >= 0 | abs(Likelihood_result_next-Likelihood_result_before) < 0.01)           //����Ȼ���������ν��С����ֵ������Ȼ�����������������
	{

		for (int xi = 0; xi < Final_Sample; xi++){
			for (int k = 0; k < Gaussian_Num; k++){

				Gama_temp[xi][k] = Weight[k] * getGaussRatio(EM_AVE_SD[k].EM_ave, EM_AVE_SD[k].EM_sd , Sample_Flow[xi]);
				Pro_SUM += Gama_temp[xi][k];

			}

			for (int k = 0; k < Gaussian_Num; k++){
				Gama[xi][k] = Gama_temp[xi][k]/Pro_SUM;
			}

			Pro_SUM = 0;
		}

		//M������֪�ֲ�����������ĵ�����ʽ����Ȩ�ء����ֲ���ֵ�ͷ���ֵ
		for (int k = 0; k < Gaussian_Num; k++){             
			for (int xi = 0; xi < Final_Sample; xi++){
				Weight_temp[k] += Gama[xi][k];
			}

			Weight[k] = Weight_temp[k]/Final_Sample;
		}

		//���㺯���ľ�ֵ�ͷ���
		float volume_sum=0.0,weight_sum=0.0,sd_sum=0.0;
		for (int k = 0; k < Gaussian_Num; k++){
			for (int xi = 0; xi < Final_Sample; xi++){
				volume_sum=Gama[xi][k]*Flow_Predict.FLOW_VALUE[xi];

			}
			EM_AVE_SD[k].EM_ave = volume_sum/Weight_temp[k];

			for (int xi = 0; xi < Final_Sample; xi++){
				sd_sum = Gama[xi][k] * pow((Flow_Predict.FLOW_VALUE[xi]-EM_AVE_SD[k].EM_ave)*1.0,2);

			}
			EM_AVE_SD[k].EM_sd = sd_sum/Weight_temp[k];

			volume_sum=0;
			sd_sum=0;

		}

		//���������ֲ�������Ȩ��ϵ�����뼫����Ȼ���� 
		float Like_temp = 0.0;
		for (int k = 0; k < Gaussian_Num; k++){
			for (int xi = 0; xi < Final_Sample; xi++){
				Like_temp += Gama[xi][k] * ( log(1/sqrt(2*PI)) - log(1.0*EM_AVE_SD[k].EM_sd) - pow((Flow_Predict.FLOW_VALUE[xi] - EM_AVE_SD[k].EM_ave)*1.0,2) / (2*pow(1.0*EM_AVE_SD[k].EM_sd,2)) );

			}
			Like_temp = Like_temp + Weight_temp[k] * log(Weight[k]);

		}

		Likelihood_result_before = Likelihood_result_next;
		Likelihood_result_next = Like_temp;

		Cal_Times--;

	}   //while���


	//�����˹���ģ�͵ľ�ֵ�ͷ���

	for (int k = 0; k < Gaussian_Num; k++){
		Flow_EM_output->EM_ave += Weight[k] * EM_AVE_SD[k].EM_ave;
	}

	for (int k = 0; k < Gaussian_Num; k++){
		Flow_EM_output->EM_sd += pow((Weight[k] * EM_AVE_SD[k].EM_sd) ,2);
	}

	//return EM_result;

}

float getGaussRatio(float u , float sigma , int x)
{
	double fenMu=sqrt(2*PI)*sigma;
	double zhiShu=-(x-u)*(x-u)/(2*sigma*sigma);
	double fenZi=exp(zhiShu);
	return fenZi/fenMu;

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


int Data_Latency(VspdCToMySQL *vspdctomysql,string vehicle_pass_table_name,string OWNER_CROSS, string KKBH, string date_start, string date_end,char *split2)       //�����ӳ�
{
	char SQL[1024];
	char* Msg = "";//��Ϣ����
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
		//��������ӳ�ʱ��ȡƽ��
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

	//�Ѵ���ĳһEND_TIME�������DATA_MISS����������µļ�¼
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


void Data_Anomaly_HMM_judge(VspdCToMySQL *vspdctomysql, string OWNER_CROSS, string DIRECTION, string LANE_CODE, int Lane_Flow, string END_TIME_downrange, string END_TIME,  char *split2)  //�����쳣��д�뺯��
{
	char *Msg="";
	char SQL[1024];

	float Flow_Ave=0.0, Flow_Var=0.0;
	int Data_Anomaly_Index=0;

	sprintf(SQL,"select FLOW_AVE,FLOW_VAR from TB_FLOW_LIMIT_HMM where LANE_CODE='%s' and END_TIME='%s'",LANE_CODE.c_str(),END_TIME.c_str());
	string SP = vspdctomysql->SelectData(SQL,2,Msg);

	char *str = (char *)SP.c_str();
	str = strtok(str,split2);
	sscanf(str,"%f",&Flow_Ave);

	str = strtok(NULL,split2);
	sscanf(str,"%f",&Flow_Var);

	if (Lane_Flow<(Flow_Ave-Confidence_Interval*Flow_Var) | Lane_Flow>(Flow_Ave+Confidence_Interval*Flow_Var)){
		Data_Anomaly_Index = 1;
	}

	sprintf(SQL,"insert into TB_DATA_ANOMALY(LANE_CODE,OWNER_CROSS,DIRECTION,END_TIME,DATA_ANOMALY,TIME_INTERVAL) values('%s','%s','%s','%s',%d,%d)",LANE_CODE.c_str(),OWNER_CROSS.c_str(),DIRECTION.c_str(),END_TIME.c_str(),Data_Anomaly_Index,Time_Interval_MISS);
	vspdctomysql->InsertData(SQL,Msg);

}


void Data_Anomaly_SA_judge(VspdCToMySQL *vspdctomysql, string OWNER_CROSS, string DIRECTION, string LANE_CODE, int Lane_Flow, string END_TIME_downrange, string END_TIME,  char *split2)  //�����쳣��д�뺯��
{
	char *Msg="";
	char SQL[1024];

	float Flow_Ave=0.0, Flow_Var=0.0;
	int Data_Anomaly_Index=0;

	sprintf(SQL,"select FLOW_AVE,FLOW_VAR from TB_FLOW_LIMIT_SA where LANE_CODE='%s' and END_DATE=date_format('%s','%%%%T')",LANE_CODE.c_str(),END_TIME.c_str());
	string SP = vspdctomysql->SelectData(SQL,2,Msg);

	char *str = (char *)SP.c_str();
	str = strtok(str,split2);
	sscanf(str,"%f",&Flow_Ave);

	str = strtok(NULL,split2);
	sscanf(str,"%f",&Flow_Var);

	if (Lane_Flow<(Flow_Ave-Confidence_Interval*Flow_Var) | Lane_Flow>(Flow_Ave+Confidence_Interval*Flow_Var)){
		Data_Anomaly_Index = 1;
	}

	sprintf(SQL,"insert into TB_DATA_ANOMALY(LANE_CODE,OWNER_CROSS,DIRECTION,END_TIME,DATA_ANOMALY,TIME_INTERVAL) values('%s','%s','%s','%s',%d,%d)",LANE_CODE.c_str(),OWNER_CROSS.c_str(),DIRECTION.c_str(),END_TIME.c_str(),Data_Anomaly_Index,Time_Interval_MISS);
	vspdctomysql->InsertData(SQL,Msg);

}

string Get_Date(){

	char local_time[24];

	SYSTEMTIME st = { 0 };//20180306
	GetLocalTime(&st);
	sprintf(local_time, "%d-%02d-%02d %02d:%02d:%02d", st.wYear,
		st.wMonth,
		st.wDay,
		st.wHour,
		st.wMinute,
		st.wSecond);

	return local_time;

}

