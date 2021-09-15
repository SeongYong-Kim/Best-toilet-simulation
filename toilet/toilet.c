/* External definitions for multiteller bank. */

#include "simlib.h"             /* Required for use of simlib.c. */

#define EVENT_ARRIVAL_HS     1  /* Event type for 하스 arrival of a customer. */
#define EVENT_ARRIVAL_SLU    2  /* Event type for 과도 지하 arrival of a customer. */
#define EVENT_ARRIVAL_SL1    3  /* Event type for 과도 1층 arrival of a customer. */
#define EVENT_DEPARTURE_HS   4  /* Event type for 하스 departure of a customer. */
#define EVENT_DEPARTURE_SLU  5  /* Event type for 과도 지하 departure of a customer. */
#define EVENT_DEPARTURE_SL1  6  /* Event type for 과도 1층departure of a customer. */
#define EVENT_CLOSE_DOORS    7  /* Event type for closing doors at 5 P.M. */
#define EVENT_ARRIVAL_HS_J   8  /* Event type for 하스 arrival of a customer. */
#define EVENT_ARRIVAL_SLU_J  9  /* Event type for 과도 지하 arrival of a customer. */
#define SAMPST_DELAYS        1  /* sampst variable for delays in queue(s). */
#define STREAM_INTERARRIVAL_HS  1  /* Random-number stream for 하스 interarrivals. */
#define STREAM_INTERARRIVAL_SLU 1  /* Random-number stream for 과도 지하 interarrivals. */
#define STREAM_INTERARRIVAL_SL1 1  /* Random-number stream for 과도 1층 interarrivals. */
#define STREAM_SERVICE_HS       1  /* Random-number stream for service times. */
#define STREAM_SERVICE_SLU      1  /* Random-number stream for service times. */
#define STREAM_SERVICE_SL1      1  /* Random-number stream for service times. */


/* Declare non-simlib global variables. */

int   num_tellers, shortest_length, shortest_queue, HS_server, SL1_server, SLU_server, total_servers, condition, alter_HS, alter_SLU, alter_SL1, num_joc;
float mean_interarrival_HS, mean_interarrival_SL1, mean_interarrival_SLU, mean_service, length_doors_open, time_last_event,
      area_server_status_HS, area_server_status_SLU, area_server_status_SL1, server_status_HS, server_status_SLU, server_status_SL1;

FILE  *infile, *outfile;

/* Declare non-simlib functions. */

void arrive_HS(void);
void arrive_SLU(void);
void arrive_SL1(void);
void depart_HS(int teller);
void depart_SLU(int teller);
void depart_SL1(int teller);
void jockey_in_HS(int teller);
void update_time_avg_stats(void);
void report(void);



main()  /* Main function. */
{
    /* Open input and output files. */

    infile  = fopen("mtbank.in",  "r");
    outfile = fopen("mtbank.out", "w");

    /* Read input parameters. */

    fscanf(infile, "%d %d %d %d %f %f %f %f %f", &HS_server, &SLU_server, &SL1_server, &total_servers,
           &mean_interarrival_HS, &mean_interarrival_SLU, &mean_interarrival_SL1, &mean_service, &length_doors_open);


    /* Write report heading and input parameters. */

    fprintf(outfile, "Toilets with separate queues & jockeying\n\n");
    fprintf(outfile,
            "Toilets closes after%16.3f hours\n\n\n\n", length_doors_open);

    num_tellers = total_servers;

    alter_SLU = SLU_server;
    alter_SL1 = SL1_server-1;

    /* Initialize simlib */
    for (alter_HS = HS_server; alter_HS >= HS_server-2; --alter_HS) {
        ++alter_SL1;
        time_last_event = 0.0;
        server_status_HS = 0.0;
        server_status_SLU = 0.0;
        server_status_SL1 = 0.0;
        area_server_status_HS = 0.0;
        area_server_status_SL1 = 0.0;
        area_server_status_SLU = 0.0;
        num_joc = 0;

        init_simlib();

        /* Set maxatr = max(maximum number of attributes per record, 4) */

        maxatr = 5;  /* NEVER SET maxatr TO BE SMALLER THAN 4. */

        /* Schedule the first arrival. */

        event_schedule(gamma(221.33, 2.2195, STREAM_INTERARRIVAL_HS),
            EVENT_ARRIVAL_HS);

        event_schedule(weibull(687.82, 2.6594, 3.109, STREAM_INTERARRIVAL_SLU),
            EVENT_ARRIVAL_SLU);

        event_schedule(lognormal(463.19, 4.1158, STREAM_INTERARRIVAL_SL1),
            EVENT_ARRIVAL_SL1);


        /* Schedule the bank closing.  (Note need for consistency of units.) */

        event_schedule(60 * length_doors_open, EVENT_CLOSE_DOORS);

        /* Run the simulation while the event list is not empty. */
        condition = 0;
        while (list_size[LIST_EVENT] != 0) {
            /* Determine the next event. */
        //while (condition == "end") {
            timing();
            update_time_avg_stats();

                   /* Invoke the appropriate event function. */

            switch (next_event_type) {

            case EVENT_ARRIVAL_HS:
                printf("arrive_HS\n");
                arrive_HS();
                break;
            case EVENT_ARRIVAL_HS_J:
                printf("arrive_HS_J\n");
                arrive_HS();
                break;
            case EVENT_ARRIVAL_SLU:
                printf("arrive_SLU\n");
                arrive_SLU();
                break;
            case EVENT_ARRIVAL_SLU_J:
                printf("arrive_SLU_J\n");
                arrive_SLU();
                break;
            case EVENT_ARRIVAL_SL1:
                printf("arrive_SL1\n");
                arrive_SL1();
                break;
            case EVENT_DEPARTURE_HS:
                printf("depart_HS\n");
                depart_HS((int)transfer[3]);  /* transfer[3] is teller number. */
                break;
            case EVENT_DEPARTURE_SLU:
                printf("depart_SLU\n");
                depart_SLU((int)transfer[3]);  /* transfer[3] is teller number. */
                break;
            case EVENT_DEPARTURE_SL1:
                printf("depart_SL1\n");
                depart_SL1((int)transfer[3]);  /* transfer[3] is teller number. */
                break;
            case EVENT_CLOSE_DOORS:
                printf("------------------------END-------------------------");
                condition = 1;
                event_cancel(EVENT_ARRIVAL_HS);
                event_cancel(EVENT_ARRIVAL_SLU);
                event_cancel(EVENT_ARRIVAL_SL1);
                break;
            }
        }

        /* Report results for the simulation with num_tellers tellers. */
        report();
    }

    alter_SLU = SLU_server + 3;
    alter_SL1 = SL1_server;

    for (alter_HS = HS_server-1; alter_HS <= HS_server + 1; alter_HS += 2) {
        alter_SLU -= 2;
        time_last_event = 0.0;
        server_status_HS = 0.0;
        server_status_SLU = 0.0;
        server_status_SL1 = 0.0;
        area_server_status_HS = 0.0;
        area_server_status_SL1 = 0.0;
        area_server_status_SLU = 0.0;
        num_joc = 0;

        init_simlib();

        /* Set maxatr = max(maximum number of attributes per record, 4) */

        maxatr = 5;  /* NEVER SET maxatr TO BE SMALLER THAN 4. */

        /* Schedule the first arrival. */

        event_schedule(gamma(221.33, 2.2195, STREAM_INTERARRIVAL_HS),
            EVENT_ARRIVAL_HS);

        event_schedule(weibull(687.82, 2.6594, 3.109, STREAM_INTERARRIVAL_SLU),
            EVENT_ARRIVAL_SLU);

        event_schedule(lognormal(463.19, 4.1158, STREAM_INTERARRIVAL_SL1),
            EVENT_ARRIVAL_SL1);


        /* Schedule the bank closing.  (Note need for consistency of units.) */

        event_schedule(60 * length_doors_open, EVENT_CLOSE_DOORS);

        /* Run the simulation while the event list is not empty. */
        condition = 0;
        while (list_size[LIST_EVENT] != 0) {
            /* Determine the next event. */
        //while (condition == "end") {
            timing();
            update_time_avg_stats();

            //       printf("%f", generalized_pareto(-0.28246, 366.29,337.75, 1));
                   /* Invoke the appropriate event function. */

            switch (next_event_type) {

            case EVENT_ARRIVAL_HS:
                printf("arrive_HS\n");
                arrive_HS();
                break;
            case EVENT_ARRIVAL_HS_J:
                printf("arrive_HS_J\n");
                arrive_HS();
                break;
            case EVENT_ARRIVAL_SLU:
                printf("arrive_SLU\n");
                arrive_SLU();
                break;
            case EVENT_ARRIVAL_SLU_J:
                printf("arrive_SLU_J\n");
                arrive_SLU();
                break;
            case EVENT_ARRIVAL_SL1:
                printf("arrive_SL1\n");
                arrive_SL1();
                break;
            case EVENT_DEPARTURE_HS:
                printf("depart_HS\n");
                depart_HS((int)transfer[3]);  /* transfer[3] is teller number. */
                break;
            case EVENT_DEPARTURE_SLU:
                printf("depart_SLU\n");
                depart_SLU((int)transfer[3]);  /* transfer[3] is teller number. */
                break;
            case EVENT_DEPARTURE_SL1:
                printf("depart_SL1\n");
                depart_SL1((int)transfer[3]);  /* transfer[3] is teller number. */
                break;
            case EVENT_CLOSE_DOORS:
                printf("------------------------END-------------------------");
                condition = 1;
                event_cancel(EVENT_ARRIVAL_HS);
                event_cancel(EVENT_ARRIVAL_SLU);
                event_cancel(EVENT_ARRIVAL_SL1);
                break;
            }
        }

        /* Report results for the simulation with num_tellers tellers. */
        report();
    }

    fclose(infile);
    fclose(outfile);

    return 0;
}


void arrive_HS(void)  /* Event function for arrival of a customer to the bank. */
{
    if (condition == 1) {
        return; /*condition == "end"이면 작동 멈춤*/
    }
        int teller;

    /* Schedule next arrival. */
    if (next_event_type == EVENT_ARRIVAL_HS) {
        event_schedule(sim_time + gamma(221.33, 2.2195, STREAM_INTERARRIVAL_HS),
            EVENT_ARRIVAL_HS);
    }

    /* If a teller is idle, start service on the arriving customer. */

    for (teller = 1; teller <= num_tellers-(alter_SLU + alter_SL1); ++teller) {

        if (list_size[num_tellers + teller] == 0) {

            /* This teller is idle, so customer has delay of zero. */

            sampst(0.0, SAMPST_DELAYS);

            /* Make this teller busy (attributes are irrelevant). */

            list_file(FIRST, num_tellers + teller);

            /* Schedule a service completion. */

            transfer[3] = teller;  /* Define third attribute of type-two event-
                                      list record before event_schedule. */

            event_schedule(sim_time + generalized_pareto(-0.28246, 366.29, 337.75, STREAM_SERVICE_HS),
                           EVENT_DEPARTURE_HS);

            /*utilization 계산에 필요*/
            server_status_HS = 0.0;
            for (teller = 1; teller <= alter_HS; ++teller) {
                server_status_HS += list_size[num_tellers + teller];
            }

            /* Return control to the main function. */
            return;
        }
    }

    /* All tellers are busy, so find the shortest queue (leftmost shortest in
       case of ties). */

    shortest_length = list_size[1];
    shortest_queue  = 1;
    for (teller = 2; teller <= num_tellers- (alter_SL1 + alter_SLU); ++teller)
        if (list_size[teller] < shortest_length) {
            shortest_length = list_size[teller];
            shortest_queue  = teller;
        }

    /* Place the customer at the end of the leftmost shortest queue. */
    transfer[1] = sim_time;
    list_file(LAST, shortest_queue);

    /*utilization 계산에 필요*/
    server_status_HS = 0.0;
    for (teller = 1; teller <= HS_server; ++teller) {
        server_status_HS += list_size[num_tellers + teller];
    }

}

void arrive_SLU(void)  /* Event function for arrival of a customer to the bank. */
{
    if (condition == 1) {
        return; /*condition == "end"이면 작동 멈춤*/
    }

    int teller, state;
    state = 0;
    /* Schedule next arrival. */
        event_schedule(sim_time + weibull(687.82, 2.6594, 3.109, STREAM_INTERARRIVAL_SLU),
            EVENT_ARRIVAL_SLU);

    /* Check to see whether a jockeying customer was found. */

    for (teller = alter_HS+1; teller <= num_tellers - (alter_SL1); ++teller) {
        state += list_size[teller] + list_size[num_tellers + teller];
    }

    if (state >= alter_SLU) {
        /*state가 server_num 이상이면 자킹*/
        printf("jockey 과도지하 -> 하스\n");
        event_schedule(sim_time + 1, EVENT_ARRIVAL_HS_J);
        ++num_joc;
        return;
    }

    /* If a teller is idle, start service on the arriving customer. */

    for (teller = alter_HS+1; teller <= num_tellers - (alter_SL1); ++teller) {

        if (list_size[num_tellers + teller] == 0) {

            /* This teller is idle, so customer has delay of zero. */

            sampst(0.0, SAMPST_DELAYS);

            /* Make this teller busy (attributes are irrelevant). */

            list_file(FIRST, num_tellers + teller);

            /* Schedule a service completion. */

            transfer[3] = teller;  /* Define third attribute of type-two event-
                                      list record before event_schedule. */

            event_schedule(sim_time + generalized_pareto(-0.28246, 366.29, 337.75, STREAM_SERVICE_SLU),
                EVENT_DEPARTURE_SLU);

            /*utilization 계산에 필요*/
            server_status_SLU = 0.0;
            for (teller = alter_HS+1; teller <= num_tellers-alter_SL1; ++teller) {
                server_status_SLU += list_size[num_tellers + teller];
            }

            /* Return control to the main function. */

            return;
        }
    }

    /* All tellers are busy, so find the shortest queue (leftmost shortest in
       case of ties). */

    shortest_length = list_size[alter_HS + 1];
    shortest_queue = 1;
    for (teller = alter_HS+2; teller <= num_tellers - (alter_SL1); ++teller)
        if (list_size[teller] < shortest_length) {
            shortest_length = list_size[teller];
            shortest_queue = teller;
        }

    /* Place the customer at the end of the leftmost shortest queue. */

    transfer[1] = sim_time;
    list_file(LAST, shortest_queue);

    /*utilization 계산에 필요*/
    server_status_SLU = 0.0;
    for (teller = alter_HS + 1; teller <= num_tellers - alter_SL1; ++teller) {
        server_status_SLU += list_size[num_tellers + teller];
    }

}

void arrive_SL1(void)  /* Event function for arrival of a customer to the bank. */
{
    if (condition == 1) {
        return; /*condition == "end"이면 작동 멈춤*/
    }

    int teller, state;
    state = 0;

    /* Schedule next arrival. */

    event_schedule(sim_time + lognormal(463.19, 4.1158, STREAM_INTERARRIVAL_SL1),
        EVENT_ARRIVAL_SL1);


    /* Check to see whether a jockeying customer was found. */

    for (teller = alter_HS + alter_SLU+1; teller <= num_tellers; ++teller) {
        state += list_size[teller] + list_size[num_tellers + teller];
    }

    if (state >= alter_SL1) {
        printf("jockey 과도1층 -> 과도지하\n");
        /*state가 server_num 이상이면 자킹*/
        event_schedule(sim_time + 1, EVENT_ARRIVAL_SLU_J);
        ++num_joc;
        return;
    }


    /* If a teller is idle, start service on the arriving customer. */

    for (teller = alter_HS + alter_SLU+1; teller <= num_tellers; ++teller) {

        if (list_size[num_tellers + teller] == 0) {

            /* This teller is idle, so customer has delay of zero. */

            sampst(0.0, SAMPST_DELAYS);

            /* Make this teller busy (attributes are irrelevant). */

            list_file(FIRST, num_tellers + teller);

            /* Schedule a service completion. */

            transfer[3] = teller;  /* Define third attribute of type-two event-
                                      list record before event_schedule. */

            event_schedule(sim_time + generalized_pareto(-0.28246, 366.29, 337.75, STREAM_SERVICE_SL1),
                EVENT_DEPARTURE_SL1);

            /*utilization 계산에 필요*/
            server_status_SL1 = 0.0;
            for (teller = alter_HS+alter_SLU+1; teller <= num_tellers; ++teller) {
                server_status_SL1 += list_size[num_tellers + teller];
            }

            /* Return control to the main function. */

            return;
        }
    }

    /* All tellers are busy, so find the shortest queue (leftmost shortest in
       case of ties). */

    shortest_length = list_size[alter_HS+alter_SLU+1];
    shortest_queue = 1;
    for (teller = alter_HS+alter_SLU+2; teller <= num_tellers; ++teller)
        if (list_size[teller] < shortest_length) {
            shortest_length = list_size[teller];
            shortest_queue = teller;
        }

    /* Place the customer at the end of the leftmost shortest queue. */

    transfer[1] = sim_time;
    list_file(LAST, shortest_queue);

    /*utilization 계산에 필요*/
    server_status_SL1 = 0.0;
    for (teller = alter_HS + alter_SLU + 1; teller <= num_tellers; ++teller) {
        server_status_SL1 += list_size[num_tellers + teller];
    }

}


void depart_HS(int teller)  /* Departure event function. */
{
    int num;

    /* Check to see whether the queue for teller "teller" is empty. */

    if (list_size[teller] == 0)

        /* The queue is empty, so make the teller idle. */

        list_remove(FIRST, num_tellers + teller);

    else {

        /* The queue is not empty, so start service on a customer. */

        list_remove(FIRST, teller);
        sampst(sim_time - transfer[1], SAMPST_DELAYS);
        transfer[3] = teller;  /* Define before event_schedule. */
        event_schedule(sim_time + generalized_pareto(-0.28246, 366.29, 337.75, STREAM_SERVICE_HS),
                       EVENT_DEPARTURE_HS);
    }

    /*utilization 계산에 필요*/
    server_status_HS = 0.0;
    for (num = 1; num <= alter_HS; ++num) {
        server_status_HS += list_size[num_tellers + num];
    }

    /* Let a customer from the end of another queue jockey to the end of this
       queue, if possible. */

    jockey_in_HS(teller);
}

void depart_SLU(int teller)  /* Departure event function. */
{   
    int num;

    /* Check to see whether the queue for teller "teller" is empty. */

    if (list_size[teller] == 0)

        /* The queue is empty, so make the teller idle. */

        list_remove(FIRST, num_tellers + teller);

    else {

        /* The queue is not empty, so start service on a customer. */

        list_remove(FIRST, teller);
        sampst(sim_time - transfer[1], SAMPST_DELAYS);
        transfer[3] = teller;  /* Define before event_schedule. */
        event_schedule(sim_time + generalized_pareto(-0.28246, 366.29, 337.75, STREAM_SERVICE_SLU),
            EVENT_DEPARTURE_SLU);
    }
    /*utilization 계산에 필요*/
    server_status_SLU = 0.0;
    for (num = alter_HS + 1; num <= num_tellers - alter_SL1; ++num) {
        server_status_SLU += list_size[num_tellers + num];
    }

}

void depart_SL1(int teller)  /* Departure event function. */
{
    int num;

    /* Check to see whether the queue for teller "teller" is empty. */

    if (list_size[teller] == 0)

        /* The queue is empty, so make the teller idle. */

        list_remove(FIRST, num_tellers + teller);

    else {

        /* The queue is not empty, so start service on a customer. */

        list_remove(FIRST, teller);
        sampst(sim_time - transfer[1], SAMPST_DELAYS);
        transfer[3] = teller;  /* Define before event_schedule. */
        event_schedule(sim_time + generalized_pareto(-0.28246, 366.29, 337.75, STREAM_SERVICE_SL1),
            EVENT_DEPARTURE_SL1);
    }
    /*utilization 계산에 필요*/
    server_status_SL1 = 0.0;
    for (num = alter_HS + alter_SLU + 1; num <= num_tellers; ++num) {
        server_status_SL1 += list_size[num_tellers + num];
    }

}


void jockey_in_HS(int teller)  /* Jockey a customer to the end of queue "teller" from
                            the end of another queue, if possible. */
{
    int jumper, min_distance, ni, nj, other_teller, distance;
    /* Find the number, jumper, of the queue whose last customer will jockey to
       queue or teller "teller", if there is such a customer. */

    jumper       = 0;
    min_distance = 1000;
    ni           = list_size[teller] + list_size[num_tellers + teller];

    /* Scan all the queues from left to right. */

    for (other_teller = 1; other_teller <= num_tellers-(alter_SLU+alter_SL1); ++other_teller) {

        nj = list_size[other_teller] + list_size[num_tellers + other_teller];
        distance = abs(teller - other_teller);

        /* Check whether the customer at the end of queue other_teller qualifies
           for being the jockeying choice so far. */

        if (other_teller != teller && nj > ni + 1 && distance < min_distance) {

            /* The customer at the end of queue other_teller is our choice so
               far for the jockeying customer, so remember his queue number and
               its distance from the destination queue. */
            jumper       = other_teller;
            min_distance = distance;
        }
    }

    /* Check to see whether a jockeying customer was found. */

    if (jumper > 0) {

        /* A jockeying customer was found, so remove him from his queue. */

        list_remove(LAST, jumper);

        /* Check to see whether the teller of his new queue is busy. */

        if (list_size[num_tellers + teller] > 0)

            /* The teller of his new queue is busy, so place the customer at the
               end of this queue. */

            list_file(LAST, teller);

        else {

            /* The teller of his new queue is idle, so tally the jockeying
               customer's delay, make the teller busy, and start service. */

            sampst(sim_time - transfer[1], SAMPST_DELAYS);
            list_file(FIRST, num_tellers + teller);
            transfer[3] = teller;  /* Define before event_schedule. */
            event_schedule(sim_time + generalized_pareto(-0.28246, 366.29, 337.75, STREAM_SERVICE_HS),
                           EVENT_DEPARTURE_HS);
        }
    }
}

void update_time_avg_stats(void)  /* Update area accumulators for time-average
                                     statistics. */
{
    float time_since_last_event;

    /* Compute time since last event, and update last-event-time marker. */

    time_since_last_event = sim_time - time_last_event;
    time_last_event = sim_time;

    /* Update area under server-busy indicator function. */

    area_server_status_HS  += server_status_HS  * time_since_last_event;
    area_server_status_SLU += server_status_SLU * time_since_last_event;
    area_server_status_SL1 += server_status_SL1 * time_since_last_event;

}

void report(void)  /* Report generator function. */
{
    int   teller;
    float avg_num_in_queue;

    /* Compute and write out estimates of desired measures of performance. */

    avg_num_in_queue = 0.0;
    for (teller = 1; teller <= num_tellers-(alter_SLU+alter_SL1); ++teller)
        avg_num_in_queue += filest(teller);
    fprintf(outfile, "<Result>\n\n");
    fprintf(outfile, "Server numbers  =>  HS_server : %d      SLU_server : %d      SL1_server : %d\n\n", alter_HS, alter_SLU, alter_SL1);
    fprintf(outfile, "\n\nWith %2d toilets, average number in queue = %10.3f",
            num_tellers, avg_num_in_queue);
    fprintf(outfile, "\n\nDelays in queue, in minutes:\n");
    out_sampst(outfile, SAMPST_DELAYS, SAMPST_DELAYS);

    fprintf(outfile, "Jockey average time%23.3f\n\n", num_joc/sim_time);
    fprintf(outfile, "하나스퀘어 Server utilization%13.3f\n\n", (area_server_status_HS / (alter_HS * sim_time)));
    fprintf(outfile, "과학도서관 지하 Server utilization%8.3f\n\n", (area_server_status_SLU / (alter_SLU * sim_time)));
    fprintf(outfile, "과학도서관 1층 Server utilization%9.3f\n\n", (area_server_status_SL1 / (alter_SL1 * sim_time)));
    fprintf(outfile, "Time simulation ended%23.3f minutes\n\n\n\n\n\n", sim_time);

}
