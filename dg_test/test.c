   void test(int *A) {  
     for (int i = 0; i < 100; i++) {
       int x = A[i];
       A[i - 1] = x - 1;
   //    A[i] = x + 1;
       A[i + 1] = x + 2;
       A[i + 2] = x + 4;
     }
   }

   //hand calculation show that statement 2 which has store to A[i-1] has only dependencies into it. which means all edges are negative using it as a source but positive using it as destination. Hence, hand calculation show that this statement can be put into a second, separate loop and has no effect to the final results. If dependence direction is < or = then that is true dependency from src to dst or dependence distance >= 0
