// Number of sweep counts
// TODO (Exercise 2-1): Choose an appropriate value!
let P = 25;
let N = 32768;

// Number of elements in your trace
let K = 5 * 1000 / P; 

// Array of length K with your trace's values
let T;

// Value of performance.now() when you started recording your trace
let start;

function record() {
  // Create empty array for saving trace values
  T = new Array(K);

  // Fill array with -1 so we can be sure memory is allocated
  T.fill(-1, 0, T.length);

  // Save start timestamp
  start = performance.now();

  // TODO (Exercise 2-1): Record data for 5 seconds and save values to T.
  M = new Array(N * 16).fill(-1);
  for (let i = 0; i < K; i++){
    count = 0;
    P_init = performance.now();
    while (performance.now() < P_init + P){
      for (let line = 0; line < M.length / 16; line++){
        let val = M[16*line];
      }
      count += 1;
    }
    T[i] = count;
  }

  // Once done recording, send result to main thread
  postMessage(JSON.stringify(T));
}

// DO NOT MODIFY BELOW THIS LINE -- PROVIDED BY COURSE STAFF
self.onmessage = (e) => {
  if (e.data.type === "start") {
    setTimeout(record, 0);
  }
};
