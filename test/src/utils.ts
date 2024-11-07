export function genResponseData<T>(success: boolean, msg?: string, data?: T) {
  return {
    success,
    msg,
    data,
  }
}

export function sleep(ms: number) {
  return new Promise(resolve => setTimeout(resolve, ms))
}
